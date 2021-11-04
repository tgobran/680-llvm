/* returns -1 on failure, $+[0] on success */
STATIC SSize_t
S_regmatch(pTHX_ regmatch_info *reginfo, char *startpos, regnode *prog)
{
#if PERL_VERSION < 9 && !defined(PERL_CORE)
    dMY_CXT;
#endif
    dVAR;
    const bool utf8_target = reginfo->is_utf8_target;
    const U32 uniflags = UTF8_ALLOW_DEFAULT;
    REGEXP *rex_sv = reginfo->prog;
    regexp *rex = ReANY(rex_sv);
    RXi_GET_DECL(rex,rexi);
    /* the current state. This is a cached copy of PL_regmatch_state */
    regmatch_state *st;
    /* cache heavy used fields of st in registers */
    regnode *scan;
    regnode *next;
    U32 n = 0;	/* general value; init to avoid compiler warning */
    SSize_t ln = 0; /* len or last;  init to avoid compiler warning */
    char *locinput = startpos;
    char *pushinput; /* where to continue after a PUSH */
    I32 nextchr;   /* is always set to UCHARAT(locinput) */

    bool result = 0;	    /* return value of S_regmatch */
    int depth = 0;	    /* depth of backtrack stack */
    U32 nochange_depth = 0; /* depth of GOSUB recursion with nochange */
    const U32 max_nochange_depth =
        (3 * rex->nparens > MAX_RECURSE_EVAL_NOCHANGE_DEPTH) ?
        3 * rex->nparens : MAX_RECURSE_EVAL_NOCHANGE_DEPTH;
    regmatch_state *yes_state = NULL; /* state to pop to on success of
							    subpattern */
    /* mark_state piggy backs on the yes_state logic so that when we unwind 
       the stack on success we can update the mark_state as we go */
    regmatch_state *mark_state = NULL; /* last mark state we have seen */
    regmatch_state *cur_eval = NULL; /* most recent EVAL_AB state */
    struct regmatch_state  *cur_curlyx = NULL; /* most recent curlyx */
    U32 state_num;
    bool no_final = 0;      /* prevent failure from backtracking? */
    bool do_cutgroup = 0;   /* no_final only until next branch/trie entry */
    char *startpoint = locinput;
    SV *popmark = NULL;     /* are we looking for a mark? */
    SV *sv_commit = NULL;   /* last mark name seen in failure */
    SV *sv_yes_mark = NULL; /* last mark name we have seen 
                               during a successful match */
    U32 lastopen = 0;       /* last open we saw */
    bool has_cutgroup = RX_HAS_CUTGROUP(rex) ? 1 : 0;   
    SV* const oreplsv = GvSVn(PL_replgv);
    /* these three flags are set by various ops to signal information to
     * the very next op. They have a useful lifetime of exactly one loop
     * iteration, and are not preserved or restored by state pushes/pops
     */
    bool sw = 0;	    /* the condition value in (?(cond)a|b) */
    bool minmod = 0;	    /* the next "{n,m}" is a "{n,m}?" */
    int logical = 0;	    /* the following EVAL is:
				0: (?{...})
				1: (?(?{...})X|Y)
				2: (??{...})
			       or the following IFMATCH/UNLESSM is:
			        false: plain (?=foo)
				true:  used as a condition: (?(?=foo))
			    */
    PAD* last_pad = NULL;
    dMULTICALL;
    I32 gimme = G_SCALAR;
    CV *caller_cv = NULL;	/* who called us */
    CV *last_pushed_cv = NULL;	/* most recently called (?{}) CV */
    CHECKPOINT runops_cp;	/* savestack position before executing EVAL */
    U32 maxopenparen = 0;       /* max '(' index seen so far */
    int to_complement;  /* Invert the result? */
    _char_class_number classnum;
    bool is_utf8_pat = reginfo->is_utf8_pat;
    bool match = FALSE;


#ifdef DEBUGGING
    GET_RE_DEBUG_FLAGS_DECL;
#endif

    /* protect against undef(*^R) */
    SAVEFREESV(SvREFCNT_inc_simple_NN(oreplsv));

    /* shut up 'may be used uninitialized' compiler warnings for dMULTICALL */
    multicall_oldcatch = 0;
    multicall_cv = NULL;
    cx = NULL;
    PERL_UNUSED_VAR(multicall_cop);
    PERL_UNUSED_VAR(newsp);


    PERL_ARGS_ASSERT_REGMATCH;

    DEBUG_OPTIMISE_r( DEBUG_EXECUTE_r({
	    PerlIO_printf(Perl_debug_log,"regmatch start\n");
    }));

    st = PL_regmatch_state;

    /* Note that nextchr is a byte even in UTF */
    SET_nextchr;
    scan = prog;
    while (scan != NULL) {

        DEBUG_EXECUTE_r( {
	    SV * const prop = sv_newmortal();
	    regnode *rnext=regnext(scan);
	    DUMP_EXEC_POS( locinput, scan, utf8_target );
            regprop(rex, prop, scan, reginfo, NULL);
            
	    PerlIO_printf(Perl_debug_log,
		    "%3"IVdf":%*s%s(%"IVdf")\n",
		    (IV)(scan - rexi->program), depth*2, "",
		    SvPVX_const(prop),
		    (PL_regkind[OP(scan)] == END || !rnext) ? 
		        0 : (IV)(rnext - rexi->program));
	});

	next = scan + NEXT_OFF(scan);
	if (next == scan)
	    next = NULL;
	state_num = OP(scan);

      reenter_switch:
        to_complement = 0;

        SET_nextchr;
        assert(nextchr < 256 && (nextchr >= 0 || nextchr == NEXTCHR_EOS));

	switch (state_num) {
	case SBOL: /*  /^../ and /\A../  */
	    if (locinput == reginfo->strbeg)
		break;
	    sayNO;

	case MBOL: /*  /^../m  */
	    if (locinput == reginfo->strbeg ||
		(!NEXTCHR_IS_EOS && locinput[-1] == '\n'))
	    {
		break;
	    }
	    sayNO;

	case GPOS: /*  \G  */
	    if (locinput == reginfo->ganch)
		break;
	    sayNO;

	case KEEPS: /*   \K  */
	    /* update the startpoint */
	    st->u.keeper.val = rex->offs[0].start;
	    rex->offs[0].start = locinput - reginfo->strbeg;
	    PUSH_STATE_GOTO(KEEPS_next, next, locinput);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case KEEPS_next_fail:
	    /* rollback the start point change */
	    rex->offs[0].start = st->u.keeper.val;
	    sayNO_SILENT;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case MEOL: /* /..$/m  */
	    if (!NEXTCHR_IS_EOS && nextchr != '\n')
		sayNO;
	    break;

	case SEOL: /* /..$/  */
	    if (!NEXTCHR_IS_EOS && nextchr != '\n')
		sayNO;
	    if (reginfo->strend - locinput > 1)
		sayNO;
	    break;

	case EOS: /*  \z  */
	    if (!NEXTCHR_IS_EOS)
		sayNO;
	    break;

	case SANY: /*  /./s  */
	    if (NEXTCHR_IS_EOS)
		sayNO;
            goto increment_locinput;

	case CANY: /*  \C  */
	    if (NEXTCHR_IS_EOS)
		sayNO;
	    locinput++;
	    break;

	case REG_ANY: /*  /./  */
	    if ((NEXTCHR_IS_EOS) || nextchr == '\n')
		sayNO;
            goto increment_locinput;


#undef  ST
#define ST st->u.trie
        case TRIEC: /* (ab|cd) with known charclass */
            /* In this case the charclass data is available inline so
               we can fail fast without a lot of extra overhead. 
             */
            if(!NEXTCHR_IS_EOS && !ANYOF_BITMAP_TEST(scan, nextchr)) {
                DEBUG_EXECUTE_r(
                    PerlIO_printf(Perl_debug_log,
                              "%*s  %sfailed to match trie start class...%s\n",
                              REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                );
                sayNO_SILENT;
                /* NOTREACHED */
                NOT_REACHED; /* NOTREACHED */
            }
            /* FALLTHROUGH */
	case TRIE:  /* (ab|cd)  */
	    /* the basic plan of execution of the trie is:
	     * At the beginning, run though all the states, and
	     * find the longest-matching word. Also remember the position
	     * of the shortest matching word. For example, this pattern:
	     *    1  2 3 4    5
	     *    ab|a|x|abcd|abc
	     * when matched against the string "abcde", will generate
	     * accept states for all words except 3, with the longest
	     * matching word being 4, and the shortest being 2 (with
	     * the position being after char 1 of the string).
	     *
	     * Then for each matching word, in word order (i.e. 1,2,4,5),
	     * we run the remainder of the pattern; on each try setting
	     * the current position to the character following the word,
	     * returning to try the next word on failure.
	     *
	     * We avoid having to build a list of words at runtime by
	     * using a compile-time structure, wordinfo[].prev, which
	     * gives, for each word, the previous accepting word (if any).
	     * In the case above it would contain the mappings 1->2, 2->0,
	     * 3->0, 4->5, 5->1.  We can use this table to generate, from
	     * the longest word (4 above), a list of all words, by
	     * following the list of prev pointers; this gives us the
	     * unordered list 4,5,1,2. Then given the current word we have
	     * just tried, we can go through the list and find the
	     * next-biggest word to try (so if we just failed on word 2,
	     * the next in the list is 4).
	     *
	     * Since at runtime we don't record the matching position in
	     * the string for each word, we have to work that out for
	     * each word we're about to process. The wordinfo table holds
	     * the character length of each word; given that we recorded
	     * at the start: the position of the shortest word and its
	     * length in chars, we just need to move the pointer the
	     * difference between the two char lengths. Depending on
	     * Unicode status and folding, that's cheap or expensive.
	     *
	     * This algorithm is optimised for the case where are only a
	     * small number of accept states, i.e. 0,1, or maybe 2.
	     * With lots of accepts states, and having to try all of them,
	     * it becomes quadratic on number of accept states to find all
	     * the next words.
	     */

	    {
                /* what type of TRIE am I? (utf8 makes this contextual) */
                DECL_TRIE_TYPE(scan);

                /* what trie are we using right now */
		reg_trie_data * const trie
        	    = (reg_trie_data*)rexi->data->data[ ARG( scan ) ];
		HV * widecharmap = MUTABLE_HV(rexi->data->data[ ARG( scan ) + 1 ]);
                U32 state = trie->startstate;

                if (scan->flags == EXACTL || scan->flags == EXACTFLU8) {
                    _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
                    if (utf8_target
                        && UTF8_IS_ABOVE_LATIN1(nextchr)
                        && scan->flags == EXACTL)
                    {
                        /* We only output for EXACTL, as we let the folder
                         * output this message for EXACTFLU8 to avoid
                         * duplication */
                        _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput,
                                                               reginfo->strend);
                    }
                }
                if (   trie->bitmap
                    && (NEXTCHR_IS_EOS || !TRIE_BITMAP_TEST(trie, nextchr)))
                {
        	    if (trie->states[ state ].wordnum) {
        	         DEBUG_EXECUTE_r(
                            PerlIO_printf(Perl_debug_log,
                        	          "%*s  %smatched empty string...%s\n",
                        	          REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                        );
			if (!trie->jump)
			    break;
        	    } else {
        	        DEBUG_EXECUTE_r(
                            PerlIO_printf(Perl_debug_log,
                        	          "%*s  %sfailed to match trie start class...%s\n",
                        	          REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5])
                        );
        	        sayNO_SILENT;
        	   }
                }

            { 
		U8 *uc = ( U8* )locinput;

		STRLEN len = 0;
		STRLEN foldlen = 0;
		U8 *uscan = (U8*)NULL;
		U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
		U32 charcount = 0; /* how many input chars we have matched */
		U32 accepted = 0; /* have we seen any accepting states? */

		ST.jump = trie->jump;
		ST.me = scan;
		ST.firstpos = NULL;
		ST.longfold = FALSE; /* char longer if folded => it's harder */
		ST.nextword = 0;

		/* fully traverse the TRIE; note the position of the
		   shortest accept state and the wordnum of the longest
		   accept state */

		while ( state && uc <= (U8*)(reginfo->strend) ) {
                    U32 base = trie->states[ state ].trans.base;
                    UV uvc = 0;
                    U16 charid = 0;
		    U16 wordnum;
                    wordnum = trie->states[ state ].wordnum;

		    if (wordnum) { /* it's an accept state */
			if (!accepted) {
			    accepted = 1;
			    /* record first match position */
			    if (ST.longfold) {
				ST.firstpos = (U8*)locinput;
				ST.firstchars = 0;
			    }
			    else {
				ST.firstpos = uc;
				ST.firstchars = charcount;
			    }
			}
			if (!ST.nextword || wordnum < ST.nextword)
			    ST.nextword = wordnum;
			ST.topword = wordnum;
		    }

		    DEBUG_TRIE_EXECUTE_r({
		                DUMP_EXEC_POS( (char *)uc, scan, utf8_target );
			        PerlIO_printf( Perl_debug_log,
			            "%*s  %sState: %4"UVxf" Accepted: %c ",
			            2+depth * 2, "", PL_colors[4],
			            (UV)state, (accepted ? 'Y' : 'N'));
		    });

		    /* read a char and goto next state */
		    if ( base && (foldlen || uc < (U8*)(reginfo->strend))) {
			I32 offset;
			REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc,
					     uscan, len, uvc, charid, foldlen,
					     foldbuf, uniflags);
			charcount++;
			if (foldlen>0)
			    ST.longfold = TRUE;
			if (charid &&
			     ( ((offset =
			      base + charid - 1 - trie->uniquecharcount)) >= 0)

			     && ((U32)offset < trie->lasttrans)
			     && trie->trans[offset].check == state)
			{
			    state = trie->trans[offset].next;
			}
			else {
			    state = 0;
			}
			uc += len;

		    }
		    else {
			state = 0;
		    }
		    DEBUG_TRIE_EXECUTE_r(
		        PerlIO_printf( Perl_debug_log,
		            "Charid:%3x CP:%4"UVxf" After State: %4"UVxf"%s\n",
		            charid, uvc, (UV)state, PL_colors[5] );
		    );
		}
		if (!accepted)
		   sayNO;

		/* calculate total number of accept states */
		{
		    U16 w = ST.topword;
		    accepted = 0;
		    while (w) {
			w = trie->wordinfo[w].prev;
			accepted++;
		    }
		    ST.accepted = accepted;
		}

		DEBUG_EXECUTE_r(
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sgot %"IVdf" possible matches%s\n",
			REPORT_CODE_OFF + depth * 2, "",
			PL_colors[4], (IV)ST.accepted, PL_colors[5] );
		);
		goto trie_first_try; /* jump into the fail handler */
	    }}
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case TRIE_next_fail: /* we failed - try next alternative */
        {
            U8 *uc;
            if ( ST.jump) {
                REGCP_UNWIND(ST.cp);
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
	    }
	    if (!--ST.accepted) {
	        DEBUG_EXECUTE_r({
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sTRIE failed...%s\n",
			REPORT_CODE_OFF+depth*2, "", 
			PL_colors[4],
			PL_colors[5] );
		});
		sayNO_SILENT;
	    }
	    {
		/* Find next-highest word to process.  Note that this code
		 * is O(N^2) per trie run (O(N) per branch), so keep tight */
		U16 min = 0;
		U16 word;
		U16 const nextword = ST.nextword;
		reg_trie_wordinfo * const wordinfo
		    = ((reg_trie_data*)rexi->data->data[ARG(ST.me)])->wordinfo;
		for (word=ST.topword; word; word=wordinfo[word].prev) {
		    if (word > nextword && (!min || word < min))
			min = word;
		}
		ST.nextword = min;
	    }

          trie_first_try:
            if (do_cutgroup) {
                do_cutgroup = 0;
                no_final = 0;
            }

            if ( ST.jump) {
                ST.lastparen = rex->lastparen;
                ST.lastcloseparen = rex->lastcloseparen;
	        REGCP_SET(ST.cp);
            }

	    /* find start char of end of current word */
	    {
		U32 chars; /* how many chars to skip */
		reg_trie_data * const trie
		    = (reg_trie_data*)rexi->data->data[ARG(ST.me)];

		assert((trie->wordinfo[ST.nextword].len - trie->prefixlen)
			    >=  ST.firstchars);
		chars = (trie->wordinfo[ST.nextword].len - trie->prefixlen)
			    - ST.firstchars;
		uc = ST.firstpos;

		if (ST.longfold) {
		    /* the hard option - fold each char in turn and find
		     * its folded length (which may be different */
		    U8 foldbuf[UTF8_MAXBYTES_CASE + 1];
		    STRLEN foldlen;
		    STRLEN len;
		    UV uvc;
		    U8 *uscan;

		    while (chars) {
			if (utf8_target) {
			    uvc = utf8n_to_uvchr((U8*)uc, UTF8_MAXLEN, &len,
						    uniflags);
			    uc += len;
			}
			else {
			    uvc = *uc;
			    uc++;
			}
			uvc = to_uni_fold(uvc, foldbuf, &foldlen);
			uscan = foldbuf;
			while (foldlen) {
			    if (!--chars)
				break;
			    uvc = utf8n_to_uvchr(uscan, UTF8_MAXLEN, &len,
					    uniflags);
			    uscan += len;
			    foldlen -= len;
			}
		    }
		}
		else {
		    if (utf8_target)
			while (chars--)
			    uc += UTF8SKIP(uc);
		    else
			uc += chars;
		}
	    }

	    scan = ST.me + ((ST.jump && ST.jump[ST.nextword])
			    ? ST.jump[ST.nextword]
			    : NEXT_OFF(ST.me));

	    DEBUG_EXECUTE_r({
		PerlIO_printf( Perl_debug_log,
		    "%*s  %sTRIE matched word #%d, continuing%s\n",
		    REPORT_CODE_OFF+depth*2, "", 
		    PL_colors[4],
		    ST.nextword,
		    PL_colors[5]
		    );
	    });

	    if (ST.accepted > 1 || has_cutgroup) {
		PUSH_STATE_GOTO(TRIE_next, scan, (char*)uc);
                /* NOTREACHED */
		NOT_REACHED; /* NOTREACHED */
	    }
	    /* only one choice left - just continue */
	    DEBUG_EXECUTE_r({
		AV *const trie_words
		    = MUTABLE_AV(rexi->data->data[ARG(ST.me)+TRIE_WORDS_OFFSET]);
		SV ** const tmp = trie_words
                        ? av_fetch(trie_words, ST.nextword - 1, 0) : NULL;
		SV *sv= tmp ? sv_newmortal() : NULL;

		PerlIO_printf( Perl_debug_log,
		    "%*s  %sonly one match left, short-circuiting: #%d <%s>%s\n",
		    REPORT_CODE_OFF+depth*2, "", PL_colors[4],
		    ST.nextword,
		    tmp ? pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 0,
			    PL_colors[0], PL_colors[1],
			    (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0)|PERL_PV_ESCAPE_NONASCII
			) 
		    : "not compiled under -Dr",
		    PL_colors[5] );
	    });

	    locinput = (char*)uc;
	    continue; /* execute rest of RE */
            /* NOTREACHED */
        }
#undef  ST

	case EXACTL:             /*  /abc/l       */
            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;

            /* Complete checking would involve going through every character
             * matched by the string to see if any is above latin1.  But the
             * comparision otherwise might very well be a fast assembly
             * language routine, and I (khw) don't think slowing things down
             * just to check for this warning is worth it.  So this just checks
             * the first character */
            if (utf8_target && UTF8_IS_ABOVE_LATIN1(*locinput)) {
                _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput, reginfo->strend);
            }
            /* FALLTHROUGH */
	case EXACT: {            /*  /abc/        */
	    char *s = STRING(scan);
	    ln = STR_LEN(scan);
	    if (utf8_target != is_utf8_pat) {
		/* The target and the pattern have differing utf8ness. */
		char *l = locinput;
		const char * const e = s + ln;

		if (utf8_target) {
                    /* The target is utf8, the pattern is not utf8.
                     * Above-Latin1 code points can't match the pattern;
                     * invariants match exactly, and the other Latin1 ones need
                     * to be downgraded to a single byte in order to do the
                     * comparison.  (If we could be confident that the target
                     * is not malformed, this could be refactored to have fewer
                     * tests by just assuming that if the first bytes match, it
                     * is an invariant, but there are tests in the test suite
                     * dealing with (??{...}) which violate this) */
		    while (s < e) {
			if (l >= reginfo->strend
                            || UTF8_IS_ABOVE_LATIN1(* (U8*) l))
                        {
                            sayNO;
                        }
                        if (UTF8_IS_INVARIANT(*(U8*)l)) {
			    if (*l != *s) {
                                sayNO;
                            }
                            l++;
                        }
                        else {
                            if (TWO_BYTE_UTF8_TO_NATIVE(*l, *(l+1)) != * (U8*) s)
                            {
                                sayNO;
                            }
                            l += 2;
                        }
			s++;
		    }
		}
		else {
		    /* The target is not utf8, the pattern is utf8. */
		    while (s < e) {
                        if (l >= reginfo->strend
                            || UTF8_IS_ABOVE_LATIN1(* (U8*) s))
                        {
                            sayNO;
                        }
                        if (UTF8_IS_INVARIANT(*(U8*)s)) {
			    if (*s != *l) {
                                sayNO;
                            }
                            s++;
                        }
                        else {
                            if (TWO_BYTE_UTF8_TO_NATIVE(*s, *(s+1)) != * (U8*) l)
                            {
                                sayNO;
                            }
                            s += 2;
                        }
			l++;
		    }
		}
		locinput = l;
	    }
            else {
                /* The target and the pattern have the same utf8ness. */
                /* Inline the first character, for speed. */
                if (reginfo->strend - locinput < ln
                    || UCHARAT(s) != nextchr
                    || (ln > 1 && memNE(s, locinput, ln)))
                {
                    sayNO;
                }
                locinput += ln;
            }
	    break;
	    }

	case EXACTFL: {          /*  /abc/il      */
	    re_fold_t folder;
	    const U8 * fold_array;
	    const char * s;
	    U32 fold_utf8_flags;

            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
            folder = foldEQ_locale;
            fold_array = PL_fold_locale;
	    fold_utf8_flags = FOLDEQ_LOCALE;
	    goto do_exactf;

        case EXACTFLU8:           /*  /abc/il; but all 'abc' are above 255, so
                                      is effectively /u; hence to match, target
                                      must be UTF-8. */
            if (! utf8_target) {
                sayNO;
            }
            fold_utf8_flags =  FOLDEQ_LOCALE | FOLDEQ_S1_ALREADY_FOLDED
                                             | FOLDEQ_S1_FOLDS_SANE;
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    goto do_exactf;

	case EXACTFU_SS:         /*  /\x{df}/iu   */
	case EXACTFU:            /*  /abc/iu      */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    fold_utf8_flags = is_utf8_pat ? FOLDEQ_S1_ALREADY_FOLDED : 0;
	    goto do_exactf;

        case EXACTFA_NO_TRIE:   /* This node only generated for non-utf8
                                   patterns */
            assert(! is_utf8_pat);
            /* FALLTHROUGH */
	case EXACTFA:            /*  /abc/iaa     */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    fold_utf8_flags = FOLDEQ_UTF8_NOMIX_ASCII;
	    goto do_exactf;

        case EXACTF:             /*  /abc/i    This node only generated for
                                               non-utf8 patterns */
            assert(! is_utf8_pat);
	    folder = foldEQ;
	    fold_array = PL_fold;
	    fold_utf8_flags = 0;

	  do_exactf:
	    s = STRING(scan);
	    ln = STR_LEN(scan);

	    if (utf8_target
                || is_utf8_pat
                || state_num == EXACTFU_SS
                || (state_num == EXACTFL && IN_UTF8_CTYPE_LOCALE))
            {
	      /* Either target or the pattern are utf8, or has the issue where
	       * the fold lengths may differ. */
		const char * const l = locinput;
		char *e = reginfo->strend;

		if (! foldEQ_utf8_flags(s, 0,  ln, is_utf8_pat,
			                l, &e, 0,  utf8_target, fold_utf8_flags))
		{
		    sayNO;
		}
		locinput = e;
		break;
	    }

	    /* Neither the target nor the pattern are utf8 */
	    if (UCHARAT(s) != nextchr
                && !NEXTCHR_IS_EOS
		&& UCHARAT(s) != fold_array[nextchr])
	    {
		sayNO;
	    }
	    if (reginfo->strend - locinput < ln)
		sayNO;
	    if (ln > 1 && ! folder(s, locinput, ln))
		sayNO;
	    locinput += ln;
	    break;
	}

	case NBOUNDL: /*  /\B/l  */
            to_complement = 1;
            /* FALLTHROUGH */

	case BOUNDL:  /*  /\b/l  */
            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;

            if (FLAGS(scan) != TRADITIONAL_BOUND) {
                if (! IN_UTF8_CTYPE_LOCALE) {
                    Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),
                                                B_ON_NON_UTF8_LOCALE_IS_WRONG);
                }
                goto boundu;
            }

	    if (utf8_target) {
		if (locinput == reginfo->strbeg)
		    ln = isWORDCHAR_LC('\n');
		else {
                    ln = isWORDCHAR_LC_utf8(reghop3((U8*)locinput, -1,
                                                        (U8*)(reginfo->strbeg)));
		}
                n = (NEXTCHR_IS_EOS)
                    ? isWORDCHAR_LC('\n')
                    : isWORDCHAR_LC_utf8((U8*)locinput);
	    }
	    else { /* Here the string isn't utf8 */
		ln = (locinput == reginfo->strbeg)
                     ? isWORDCHAR_LC('\n')
                     : isWORDCHAR_LC(UCHARAT(locinput - 1));
                n = (NEXTCHR_IS_EOS)
                    ? isWORDCHAR_LC('\n')
                    : isWORDCHAR_LC(nextchr);
	    }
#if defined(SPEC)
            if (to_complement ^ (ln == (SSize_t)n)) {
#else
            if (to_complement ^ (ln == n)) {
#endif
                sayNO;
            }
	    break;

	case NBOUND:  /*  /\B/   */
            to_complement = 1;
            /* FALLTHROUGH */

	case BOUND:   /*  /\b/   */
	    if (utf8_target) {
                goto bound_utf8;
            }
            goto bound_ascii_match_only;

	case NBOUNDA: /*  /\B/a  */
            to_complement = 1;
            /* FALLTHROUGH */

	case BOUNDA:  /*  /\b/a  */

          bound_ascii_match_only:
            /* Here the string isn't utf8, or is utf8 and only ascii characters
             * are to match \w.  In the latter case looking at the byte just
             * prior to the current one may be just the final byte of a
             * multi-byte character.  This is ok.  There are two cases:
             * 1) it is a single byte character, and then the test is doing
             *    just what it's supposed to.
             * 2) it is a multi-byte character, in which case the final byte is
             *    never mistakable for ASCII, and so the test will say it is
             *    not a word character, which is the correct answer. */
            ln = (locinput == reginfo->strbeg)
                 ? isWORDCHAR_A('\n')
                 : isWORDCHAR_A(UCHARAT(locinput - 1));
            n = (NEXTCHR_IS_EOS)
                ? isWORDCHAR_A('\n')
                : isWORDCHAR_A(nextchr);
#if defined(SPEC)
            if (to_complement ^ (ln == (SSize_t)n)) {
#else
            if (to_complement ^ (ln == n)) {
#endif
                sayNO;
            }
	    break;

	case NBOUNDU: /*  /\B/u  */
            to_complement = 1;
            /* FALLTHROUGH */

	case BOUNDU:  /*  /\b/u  */

          boundu:
	    if (utf8_target) {

              bound_utf8:
                switch((bound_type) FLAGS(scan)) {
                    case TRADITIONAL_BOUND:
                        ln = (locinput == reginfo->strbeg)
                             ? 0 /* isWORDCHAR_L1('\n') */
                             : isWORDCHAR_utf8(reghop3((U8*)locinput, -1,
                                                                (U8*)(reginfo->strbeg)));
                        n = (NEXTCHR_IS_EOS)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_utf8((U8*)locinput);
#if defined(SPEC)
                        match = cBOOL(ln != (SSize_t)n);
#else
                        match = cBOOL(ln != n);
#endif
                        break;
                    case GCB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE; /* GCB always matches at begin and
                                             end */
                        }
                        else {
                            /* Find the gcb values of previous and current
                             * chars, then see if is a break point */
                            match = isGCB(getGCB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getGCB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend));
                        }
                        break;

                    case SB_BOUND: /* Always matches at begin and end */
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isSB(getSB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getSB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          (U8*) reginfo->strend,
                                          utf8_target);
                        }
                        break;

                    case WB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isWB(WB_UNKNOWN,
                                         getWB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getWB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          (U8*) reginfo->strend,
                                          utf8_target);
                        }
                        break;
                }
	    }
	    else {  /* Not utf8 target */
                switch((bound_type) FLAGS(scan)) {
                    case TRADITIONAL_BOUND:
                        ln = (locinput == reginfo->strbeg)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_L1(UCHARAT(locinput - 1));
                        n = (NEXTCHR_IS_EOS)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_L1(nextchr);
#if defined(SPEC)
                        match = cBOOL(ln != (SSize_t)n);
#else
                        match = cBOOL(ln != n);
#endif
                        break;

                    case GCB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE; /* GCB always matches at begin and
                                             end */
                        }
                        else {  /* Only CR-LF combo isn't a GCB in 0-255
                                   range */
                            match =    UCHARAT(locinput - 1) != '\r'
                                    || UCHARAT(locinput) != '\n';
                        }
                        break;

                    case SB_BOUND: /* Always matches at begin and end */
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isSB(getSB_VAL_CP(UCHARAT(locinput -1)),
                                         getSB_VAL_CP(UCHARAT(locinput)),
                                         (U8*) reginfo->strbeg,
                                         (U8*) locinput,
                                         (U8*) reginfo->strend,
                                         utf8_target);
                        }
                        break;

                    case WB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isWB(WB_UNKNOWN,
                                         getWB_VAL_CP(UCHARAT(locinput -1)),
                                         getWB_VAL_CP(UCHARAT(locinput)),
                                         (U8*) reginfo->strbeg,
                                         (U8*) locinput,
                                         (U8*) reginfo->strend,
                                         utf8_target);
                        }
                        break;
                }
	    }

            if (to_complement ^ ! match) {
                sayNO;
            }
	    break;

	case ANYOFL:  /*  /[abc]/l      */
            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
            /* FALLTHROUGH */
	case ANYOF:  /*   /[abc]/       */
            if (NEXTCHR_IS_EOS)
                sayNO;
	    if (utf8_target) {
	        if (!reginclass(rex, scan, (U8*)locinput, (U8*)reginfo->strend,
                                                                   utf8_target))
		    sayNO;
		locinput += UTF8SKIP(locinput);
	    }
	    else {
		if (!REGINCLASS(rex, scan, (U8*)locinput))
		    sayNO;
		locinput++;
	    }
	    break;

        /* The argument (FLAGS) to all the POSIX node types is the class number
         * */

        case NPOSIXL:   /* \W or [:^punct:] etc. under /l */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXL:    /* \w or [:punct:] etc. under /l */
            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
            if (NEXTCHR_IS_EOS)
                sayNO;

            /* Use isFOO_lc() for characters within Latin1.  (Note that
             * UTF8_IS_INVARIANT works even on non-UTF-8 strings, or else
             * wouldn't be invariant) */
            if (UTF8_IS_INVARIANT(nextchr) || ! utf8_target) {
                if (! (to_complement ^ cBOOL(isFOO_lc(FLAGS(scan), (U8) nextchr)))) {
                    sayNO;
                }
            }
            else if (UTF8_IS_DOWNGRADEABLE_START(nextchr)) {
                if (! (to_complement ^ cBOOL(isFOO_lc(FLAGS(scan),
                                           (U8) TWO_BYTE_UTF8_TO_NATIVE(nextchr,
                                                            *(locinput + 1))))))
                {
                    sayNO;
                }
            }
            else { /* Here, must be an above Latin-1 code point */
                _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput, reginfo->strend);
                goto utf8_posix_above_latin1;
            }

            /* Here, must be utf8 */
            locinput += UTF8SKIP(locinput);
            break;

        case NPOSIXD:   /* \W or [:^punct:] etc. under /d */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXD:    /* \w or [:punct:] etc. under /d */
            if (utf8_target) {
                goto utf8_posix;
            }
            goto posixa;

        case NPOSIXA:   /* \W or [:^punct:] etc. under /a */

            if (NEXTCHR_IS_EOS) {
                sayNO;
            }

            /* All UTF-8 variants match */
            if (! UTF8_IS_INVARIANT(nextchr)) {
                goto increment_locinput;
            }

            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXA:    /* \w or [:punct:] etc. under /a */

          posixa:
            /* We get here through POSIXD, NPOSIXD, and NPOSIXA when not in
             * UTF-8, and also from NPOSIXA even in UTF-8 when the current
             * character is a single byte */

            if (NEXTCHR_IS_EOS
                || ! (to_complement ^ cBOOL(_generic_isCC_A(nextchr,
                                                            FLAGS(scan)))))
            {
                sayNO;
            }

            /* Here we are either not in utf8, or we matched a utf8-invariant,
             * so the next char is the next byte */
            locinput++;
            break;

        case NPOSIXU:   /* \W or [:^punct:] etc. under /u */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXU:    /* \w or [:punct:] etc. under /u */
          utf8_posix:
            if (NEXTCHR_IS_EOS) {
                sayNO;
            }

            /* Use _generic_isCC() for characters within Latin1.  (Note that
             * UTF8_IS_INVARIANT works even on non-UTF-8 strings, or else
             * wouldn't be invariant) */
            if (UTF8_IS_INVARIANT(nextchr) || ! utf8_target) {
                if (! (to_complement ^ cBOOL(_generic_isCC(nextchr,
                                                           FLAGS(scan)))))
                {
                    sayNO;
                }
                locinput++;
            }
            else if (UTF8_IS_DOWNGRADEABLE_START(nextchr)) {
                if (! (to_complement
                       ^ cBOOL(_generic_isCC(TWO_BYTE_UTF8_TO_NATIVE(nextchr,
                                                               *(locinput + 1)),
                                             FLAGS(scan)))))
                {
                    sayNO;
                }
                locinput += 2;
            }
            else {  /* Handle above Latin-1 code points */
              utf8_posix_above_latin1:
                classnum = (_char_class_number) FLAGS(scan);
                if (classnum < _FIRST_NON_SWASH_CC) {

                    /* Here, uses a swash to find such code points.  Load if if
                     * not done already */
                    if (! PL_utf8_swash_ptrs[classnum]) {
                        U8 flags = _CORE_SWASH_INIT_ACCEPT_INVLIST;
                        PL_utf8_swash_ptrs[classnum]
                                = _core_swash_init("utf8",
                                        "",
                                        &PL_sv_undef, 1, 0,
                                        PL_XPosix_ptrs[classnum], &flags);
                    }
                    if (! (to_complement
                           ^ cBOOL(swash_fetch(PL_utf8_swash_ptrs[classnum],
                                               (U8 *) locinput, TRUE))))
                    {
                        sayNO;
                    }
                }
                else {  /* Here, uses macros to find above Latin-1 code points */
                    switch (classnum) {
                        case _CC_ENUM_SPACE:
                            if (! (to_complement
                                        ^ cBOOL(is_XPERLSPACE_high(locinput))))
                            {
                                sayNO;
                            }
                            break;
                        case _CC_ENUM_BLANK:
                            if (! (to_complement
                                            ^ cBOOL(is_HORIZWS_high(locinput))))
                            {
                                sayNO;
                            }
                            break;
                        case _CC_ENUM_XDIGIT:
                            if (! (to_complement
                                            ^ cBOOL(is_XDIGIT_high(locinput))))
                            {
                                sayNO;
                            }
                            break;
                        case _CC_ENUM_VERTSPACE:
                            if (! (to_complement
                                            ^ cBOOL(is_VERTWS_high(locinput))))
                            {
                                sayNO;
                            }
                            break;
                        default:    /* The rest, e.g. [:cntrl:], can't match
                                       above Latin1 */
                            if (! to_complement) {
                                sayNO;
                            }
                            break;
                    }
                }
                locinput += UTF8SKIP(locinput);
            }
            break;

	case CLUMP: /* Match \X: logical Unicode character.  This is defined as
		       a Unicode extended Grapheme Cluster */
	    if (NEXTCHR_IS_EOS)
		sayNO;
	    if  (! utf8_target) {

		/* Match either CR LF  or '.', as all the other possibilities
		 * require utf8 */
		locinput++;	    /* Match the . or CR */
		if (nextchr == '\r' /* And if it was CR, and the next is LF,
				       match the LF */
		    && locinput < reginfo->strend
		    && UCHARAT(locinput) == '\n')
                {
                    locinput++;
                }
	    }
	    else {

                /* Get the gcb type for the current character */
                GCB_enum prev_gcb = getGCB_VAL_UTF8((U8*) locinput,
                                                       (U8*) reginfo->strend);

                /* Then scan through the input until we get to the first
                 * character whose type is supposed to be a gcb with the
                 * current character.  (There is always a break at the
                 * end-of-input) */
                locinput += UTF8SKIP(locinput);
                while (locinput < reginfo->strend) {
                    GCB_enum cur_gcb = getGCB_VAL_UTF8((U8*) locinput,
                                                         (U8*) reginfo->strend);
                    if (isGCB(prev_gcb, cur_gcb)) {
                        break;
                    }

                    prev_gcb = cur_gcb;
                    locinput += UTF8SKIP(locinput);
                }


	    }
	    break;
            
	case NREFFL:  /*  /\g{name}/il  */
	{   /* The capture buffer cases.  The ones beginning with N for the
	       named buffers just convert to the equivalent numbered and
	       pretend they were called as the corresponding numbered buffer
	       op.  */
	    /* don't initialize these in the declaration, it makes C++
	       unhappy */
	    const char *s;
	    char type;
	    re_fold_t folder;
	    const U8 *fold_array;
	    UV utf8_fold_flags;

            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
	    folder = foldEQ_locale;
	    fold_array = PL_fold_locale;
	    type = REFFL;
	    utf8_fold_flags = FOLDEQ_LOCALE;
	    goto do_nref;

	case NREFFA:  /*  /\g{name}/iaa  */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    type = REFFA;
	    utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII;
	    goto do_nref;

	case NREFFU:  /*  /\g{name}/iu  */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    type = REFFU;
	    utf8_fold_flags = 0;
	    goto do_nref;

	case NREFF:  /*  /\g{name}/i  */
	    folder = foldEQ;
	    fold_array = PL_fold;
	    type = REFF;
	    utf8_fold_flags = 0;
	    goto do_nref;

	case NREF:  /*  /\g{name}/   */
	    type = REF;
	    folder = NULL;
	    fold_array = NULL;
	    utf8_fold_flags = 0;
	  do_nref:

	    /* For the named back references, find the corresponding buffer
	     * number */
	    n = reg_check_named_buff_matched(rex,scan);

            if ( ! n ) {
                sayNO;
	    }
	    goto do_nref_ref_common;

	case REFFL:  /*  /\1/il  */
            _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
	    folder = foldEQ_locale;
	    fold_array = PL_fold_locale;
	    utf8_fold_flags = FOLDEQ_LOCALE;
	    goto do_ref;

	case REFFA:  /*  /\1/iaa  */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII;
	    goto do_ref;

	case REFFU:  /*  /\1/iu  */
	    folder = foldEQ_latin1;
	    fold_array = PL_fold_latin1;
	    utf8_fold_flags = 0;
	    goto do_ref;

	case REFF:  /*  /\1/i  */
	    folder = foldEQ;
	    fold_array = PL_fold;
	    utf8_fold_flags = 0;
	    goto do_ref;

        case REF:  /*  /\1/    */
	    folder = NULL;
	    fold_array = NULL;
	    utf8_fold_flags = 0;

	  do_ref:
	    type = OP(scan);
	    n = ARG(scan);  /* which paren pair */

	  do_nref_ref_common:
	    ln = rex->offs[n].start;
	    reginfo->poscache_iter = reginfo->poscache_maxiter; /* Void cache */
	    if (rex->lastparen < n || ln == -1)
		sayNO;			/* Do not match unless seen CLOSEn. */
	    if (ln == rex->offs[n].end)
		break;

	    s = reginfo->strbeg + ln;
	    if (type != REF	/* REF can do byte comparison */
		&& (utf8_target || type == REFFU || type == REFFL))
	    {
		char * limit = reginfo->strend;

		/* This call case insensitively compares the entire buffer
		    * at s, with the current input starting at locinput, but
                    * not going off the end given by reginfo->strend, and
                    * returns in <limit> upon success, how much of the
                    * current input was matched */
		if (! foldEQ_utf8_flags(s, NULL, rex->offs[n].end - ln, utf8_target,
				    locinput, &limit, 0, utf8_target, utf8_fold_flags))
		{
		    sayNO;
		}
		locinput = limit;
		break;
	    }

	    /* Not utf8:  Inline the first character, for speed. */
	    if (!NEXTCHR_IS_EOS &&
                UCHARAT(s) != nextchr &&
		(type == REF ||
		 UCHARAT(s) != fold_array[nextchr]))
		sayNO;
	    ln = rex->offs[n].end - ln;
	    if (locinput + ln > reginfo->strend)
		sayNO;
	    if (ln > 1 && (type == REF
			   ? memNE(s, locinput, ln)
			   : ! folder(s, locinput, ln)))
		sayNO;
	    locinput += ln;
	    break;
	}

	case NOTHING: /* null op; e.g. the 'nothing' following
                       * the '*' in m{(a+|b)*}' */
	    break;
	case TAIL: /* placeholder while compiling (A|B|C) */
	    break;

#undef  ST
#define ST st->u.eval
	{
	    SV *ret;
	    REGEXP *re_sv;
            regexp *re;
            regexp_internal *rei;
            regnode *startpoint;

	case GOSTART: /*  (?R)  */
	case GOSUB: /*    /(...(?1))/   /(...(?&foo))/   */
	    if (cur_eval && cur_eval->locinput==locinput) {
                if (cur_eval->u.eval.close_paren == (U32)ARG(scan)) 
                    Perl_croak(aTHX_ "Infinite recursion in regex");
                if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_ 
                        "Pattern subroutine nesting without pos change"
                        " exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }
	    re_sv = rex_sv;
            re = rex;
            rei = rexi;
            if (OP(scan)==GOSUB) {
                startpoint = scan + ARG2L(scan);
                ST.close_paren = ARG(scan);
            } else {
                startpoint = rei->program+1;
                ST.close_paren = 0;
            }

            /* Save all the positions seen so far. */
            ST.cp = regcppush(rex, 0, maxopenparen);
            REGCP_SET(ST.lastcp);

            /* and then jump to the code we share with EVAL */
            goto eval_recurse_doit;
            /* NOTREACHED */

        case EVAL:  /*   /(?{A})B/   /(??{A})B/  and /(?(?{A})X|Y)B/   */        
            if (cur_eval && cur_eval->locinput==locinput) {
		if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_ "EVAL without pos change exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }    
	    {
		/* execute the code in the {...} */

		dSP;
		IV before;
		OP * const oop = PL_op;
		COP * const ocurcop = PL_curcop;
		OP *nop;
		CV *newcv;

		/* save *all* paren positions */
		regcppush(rex, 0, maxopenparen);
		REGCP_SET(runops_cp);

		if (!caller_cv)
		    caller_cv = find_runcv(NULL);

		n = ARG(scan);

		if (rexi->data->what[n] == 'r') { /* code from an external qr */
		    newcv = (ReANY(
						(REGEXP*)(rexi->data->data[n])
					    ))->qr_anoncv
					;
		    nop = (OP*)rexi->data->data[n+1];
		}
		else if (rexi->data->what[n] == 'l') { /* literal code */
		    newcv = caller_cv;
		    nop = (OP*)rexi->data->data[n];
		    assert(CvDEPTH(newcv));
		}
		else {
		    /* literal with own CV */
		    assert(rexi->data->what[n] == 'L');
		    newcv = rex->qr_anoncv;
		    nop = (OP*)rexi->data->data[n];
		}

		/* normally if we're about to execute code from the same
		 * CV that we used previously, we just use the existing
		 * CX stack entry. However, its possible that in the
		 * meantime we may have backtracked, popped from the save
		 * stack, and undone the SAVECOMPPAD(s) associated with
		 * PUSH_MULTICALL; in which case PL_comppad no longer
		 * points to newcv's pad. */
		if (newcv != last_pushed_cv || PL_comppad != last_pad)
		{
                    U8 flags = (CXp_SUB_RE |
                                ((newcv == caller_cv) ? CXp_SUB_RE_FAKE : 0));
		    if (last_pushed_cv) {
			CHANGE_MULTICALL_FLAGS(newcv, flags);
		    }
		    else {
			PUSH_MULTICALL_FLAGS(newcv, flags);
		    }
		    last_pushed_cv = newcv;
		}
		else {
                    /* these assignments are just to silence compiler
                     * warnings */
		    multicall_cop = NULL;
		    newsp = NULL;
		}
		last_pad = PL_comppad;

		/* the initial nextstate you would normally execute
		 * at the start of an eval (which would cause error
		 * messages to come from the eval), may be optimised
		 * away from the execution path in the regex code blocks;
		 * so manually set PL_curcop to it initially */
		{
		    OP *o = cUNOPx(nop)->op_first;
		    assert(o->op_type == OP_NULL);
		    if (o->op_targ == OP_SCOPE) {
			o = cUNOPo->op_first;
		    }
		    else {
			assert(o->op_targ == OP_LEAVE);
			o = cUNOPo->op_first;
			assert(o->op_type == OP_ENTER);
			o = OpSIBLING(o);
		    }

		    if (o->op_type != OP_STUB) {
			assert(    o->op_type == OP_NEXTSTATE
				|| o->op_type == OP_DBSTATE
				|| (o->op_type == OP_NULL
				    &&  (  o->op_targ == OP_NEXTSTATE
					|| o->op_targ == OP_DBSTATE
					)
				    )
			);
			PL_curcop = (COP*)o;
		    }
		}
		nop = nop->op_next;

		DEBUG_STATE_r( PerlIO_printf(Perl_debug_log, 
		    "  re EVAL PL_op=0x%"UVxf"\n", PTR2UV(nop)) );

		rex->offs[0].end = locinput - reginfo->strbeg;
                if (reginfo->info_aux_eval->pos_magic)
                    MgBYTEPOS_set(reginfo->info_aux_eval->pos_magic,
                                  reginfo->sv, reginfo->strbeg,
                                  locinput - reginfo->strbeg);

                if (sv_yes_mark) {
                    SV *sv_mrk = get_sv("REGMARK", 1);
                    sv_setsv(sv_mrk, sv_yes_mark);
                }

		/* we don't use MULTICALL here as we want to call the
		 * first op of the block of interest, rather than the
		 * first op of the sub */
		before = (IV)(SP-PL_stack_base);
		PL_op = nop;
		CALLRUNOPS(aTHX);			/* Scalar context. */
		SPAGAIN;
		if ((IV)(SP-PL_stack_base) == before)
		    ret = &PL_sv_undef;   /* protect against empty (?{}) blocks. */
		else {
		    ret = POPs;
		    PUTBACK;
		}

		/* before restoring everything, evaluate the returned
		 * value, so that 'uninit' warnings don't use the wrong
		 * PL_op or pad. Also need to process any magic vars
		 * (e.g. $1) *before* parentheses are restored */

		PL_op = NULL;

                re_sv = NULL;
		if (logical == 0)        /*   (?{})/   */
		    sv_setsv(save_scalar(PL_replgv), ret); /* $^R */
		else if (logical == 1) { /*   /(?(?{...})X|Y)/    */
		    sw = cBOOL(SvTRUE(ret));
		    logical = 0;
		}
		else {                   /*  /(??{})  */
		    /*  if its overloaded, let the regex compiler handle
		     *  it; otherwise extract regex, or stringify  */
		    if (SvGMAGICAL(ret))
			ret = sv_mortalcopy(ret);
		    if (!SvAMAGIC(ret)) {
			SV *sv = ret;
			if (SvROK(sv))
			    sv = SvRV(sv);
			if (SvTYPE(sv) == SVt_REGEXP)
			    re_sv = (REGEXP*) sv;
			else if (SvSMAGICAL(ret)) {
			    MAGIC *mg = mg_find(ret, PERL_MAGIC_qr);
			    if (mg)
				re_sv = (REGEXP *) mg->mg_obj;
			}

			/* force any undef warnings here */
			if (!re_sv && !SvPOK(ret) && !SvNIOK(ret)) {
			    ret = sv_mortalcopy(ret);
			    (void) SvPV_force_nolen(ret);
			}
		    }

		}

		/* *** Note that at this point we don't restore
		 * PL_comppad, (or pop the CxSUB) on the assumption it may
		 * be used again soon. This is safe as long as nothing
		 * in the regexp code uses the pad ! */
		PL_op = oop;
		PL_curcop = ocurcop;
		S_regcp_restore(aTHX_ rex, runops_cp, &maxopenparen);
		PL_curpm = PL_reg_curpm;

		if (logical != 2)
		    break;
	    }

		/* only /(??{})/  from now on */
		logical = 0;
		{
		    /* extract RE object from returned value; compiling if
		     * necessary */

		    if (re_sv) {
			re_sv = reg_temp_copy(NULL, re_sv);
		    }
		    else {
			U32 pm_flags = 0;

			if (SvUTF8(ret) && IN_BYTES) {
			    /* In use 'bytes': make a copy of the octet
			     * sequence, but without the flag on */
			    STRLEN len;
			    const char *const p = SvPV(ret, len);
			    ret = newSVpvn_flags(p, len, SVs_TEMP);
			}
			if (rex->intflags & PREGf_USE_RE_EVAL)
			    pm_flags |= PMf_USE_RE_EVAL;

			/* if we got here, it should be an engine which
			 * supports compiling code blocks and stuff */
			assert(rex->engine && rex->engine->op_comp);
                        assert(!(scan->flags & ~RXf_PMf_COMPILETIME));
			re_sv = rex->engine->op_comp(aTHX_ &ret, 1, NULL,
				    rex->engine, NULL, NULL,
                                    /* copy /msixn etc to inner pattern */
                                    ARG2L(scan),
                                    pm_flags);

			if (!(SvFLAGS(ret)
			      & (SVs_TEMP | SVs_GMG | SVf_ROK))
			 && (!SvPADTMP(ret) || SvREADONLY(ret))) {
			    /* This isn't a first class regexp. Instead, it's
			       caching a regexp onto an existing, Perl visible
			       scalar.  */
			    sv_magic(ret, MUTABLE_SV(re_sv), PERL_MAGIC_qr, 0, 0);
			}
		    }
		    SAVEFREESV(re_sv);
		    re = ReANY(re_sv);
		}
                RXp_MATCH_COPIED_off(re);
                re->subbeg = rex->subbeg;
                re->sublen = rex->sublen;
                re->suboffset = rex->suboffset;
                re->subcoffset = rex->subcoffset;
                re->lastparen = 0;
                re->lastcloseparen = 0;
		rei = RXi_GET(re);
                DEBUG_EXECUTE_r(
                    debug_start_match(re_sv, utf8_target, locinput,
                                    reginfo->strend, "Matching embedded");
		);		
		startpoint = rei->program + 1;
               	ST.close_paren = 0; /* only used for GOSUB */
                /* Save all the seen positions so far. */
                ST.cp = regcppush(rex, 0, maxopenparen);
                REGCP_SET(ST.lastcp);
                /* and set maxopenparen to 0, since we are starting a "fresh" match */
                maxopenparen = 0;
                /* run the pattern returned from (??{...}) */

              eval_recurse_doit: /* Share code with GOSUB below this line
                            * At this point we expect the stack context to be
                            * set up correctly */

                /* invalidate the S-L poscache. We're now executing a
                 * different set of WHILEM ops (and their associated
                 * indexes) against the same string, so the bits in the
                 * cache are meaningless. Setting maxiter to zero forces
                 * the cache to be invalidated and zeroed before reuse.
		 * XXX This is too dramatic a measure. Ideally we should
                 * save the old cache and restore when running the outer
                 * pattern again */
		reginfo->poscache_maxiter = 0;

                /* the new regexp might have a different is_utf8_pat than we do */
                is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(re_sv));

		ST.prev_rex = rex_sv;
		ST.prev_curlyx = cur_curlyx;
		rex_sv = re_sv;
		SET_reg_curpm(rex_sv);
		rex = re;
		rexi = rei;
		cur_curlyx = NULL;
		ST.B = next;
		ST.prev_eval = cur_eval;
		cur_eval = st;
		/* now continue from first node in postoned RE */
		PUSH_YES_STATE_GOTO(EVAL_AB, startpoint, locinput);
                /* NOTREACHED */
		NOT_REACHED; /* NOTREACHED */
	}

	case EVAL_AB: /* cleanup after a successful (??{A})B */
	    /* note: this is called twice; first after popping B, then A */
	    rex_sv = ST.prev_rex;
            is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
	    SET_reg_curpm(rex_sv);
	    rex = ReANY(rex_sv);
	    rexi = RXi_GET(rex);
            {
                /* preserve $^R across LEAVE's. See Bug 121070. */
                SV *save_sv= GvSV(PL_replgv);
                SvREFCNT_inc(save_sv);
                regcpblow(ST.cp); /* LEAVE in disguise */
                sv_setsv(GvSV(PL_replgv), save_sv);
                SvREFCNT_dec(save_sv);
            }
	    cur_eval = ST.prev_eval;
	    cur_curlyx = ST.prev_curlyx;

	    /* Invalidate cache. See "invalidate" comment above. */
	    reginfo->poscache_maxiter = 0;
            if ( nochange_depth )
	        nochange_depth--;
	    sayYES;


	case EVAL_AB_fail: /* unsuccessfully ran A or B in (??{A})B */
	    /* note: this is called twice; first after popping B, then A */
	    rex_sv = ST.prev_rex;
            is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
	    SET_reg_curpm(rex_sv);
	    rex = ReANY(rex_sv);
	    rexi = RXi_GET(rex); 

	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex, &maxopenparen);
	    cur_eval = ST.prev_eval;
	    cur_curlyx = ST.prev_curlyx;
	    /* Invalidate cache. See "invalidate" comment above. */
	    reginfo->poscache_maxiter = 0;
	    if ( nochange_depth )
	        nochange_depth--;
	    sayNO_SILENT;
#undef ST

	case OPEN: /*  (  */
	    n = ARG(scan);  /* which paren pair */
	    rex->offs[n].start_tmp = locinput - reginfo->strbeg;
	    if (n > maxopenparen)
		maxopenparen = n;
	    DEBUG_BUFFERS_r(PerlIO_printf(Perl_debug_log,
		"rex=0x%"UVxf" offs=0x%"UVxf": \\%"UVuf": set %"IVdf" tmp; maxopenparen=%"UVuf"\n",
		PTR2UV(rex),
		PTR2UV(rex->offs),
		(UV)n,
		(IV)rex->offs[n].start_tmp,
		(UV)maxopenparen
	    ));
            lastopen = n;
	    break;

/* XXX really need to log other places start/end are set too */
#define CLOSE_CAPTURE \
    rex->offs[n].start = rex->offs[n].start_tmp; \
    rex->offs[n].end = locinput - reginfo->strbeg; \
    DEBUG_BUFFERS_r(PerlIO_printf(Perl_debug_log, \
	"rex=0x%"UVxf" offs=0x%"UVxf": \\%"UVuf": set %"IVdf"..%"IVdf"\n", \
	PTR2UV(rex), \
	PTR2UV(rex->offs), \
	(UV)n, \
	(IV)rex->offs[n].start, \
	(IV)rex->offs[n].end \
    ))

	case CLOSE:  /*  )  */
	    n = ARG(scan);  /* which paren pair */
	    CLOSE_CAPTURE;
	    if (n > rex->lastparen)
		rex->lastparen = n;
	    rex->lastcloseparen = n;
            if (cur_eval && cur_eval->u.eval.close_paren == n) {
	        goto fake_end;
	    }    
	    break;

        case ACCEPT:  /*  (*ACCEPT)  */
            if (ARG(scan)){
                regnode *cursor;
                for (cursor=scan;
                     cursor && OP(cursor)!=END; 
                     cursor=regnext(cursor)) 
                {
                    if ( OP(cursor)==CLOSE ){
                        n = ARG(cursor);
                        if ( n <= lastopen ) {
			    CLOSE_CAPTURE;
                            if (n > rex->lastparen)
                                rex->lastparen = n;
                            rex->lastcloseparen = n;
                            if ( n == ARG(scan) || (cur_eval &&
                                cur_eval->u.eval.close_paren == n))
                                break;
                        }
                    }
                }
            }
	    goto fake_end;
	    /* NOTREACHED */

	case GROUPP:  /*  (?(1))  */
	    n = ARG(scan);  /* which paren pair */
	    sw = cBOOL(rex->lastparen >= n && rex->offs[n].end != -1);
	    break;

	case NGROUPP:  /*  (?(<name>))  */
	    /* reg_check_named_buff_matched returns 0 for no match */
	    sw = cBOOL(0 < reg_check_named_buff_matched(rex,scan));
	    break;

        case INSUBP:   /*  (?(R))  */
            n = ARG(scan);
            sw = (cur_eval && (!n || cur_eval->u.eval.close_paren == n));
            break;

        case DEFINEP:  /*  (?(DEFINE))  */
            sw = 0;
            break;

	case IFTHEN:   /*  (?(cond)A|B)  */
	    reginfo->poscache_iter = reginfo->poscache_maxiter; /* Void cache */
	    if (sw)
		next = NEXTOPER(NEXTOPER(scan));
	    else {
		next = scan + ARG(scan);
		if (OP(next) == IFTHEN) /* Fake one. */
		    next = NEXTOPER(NEXTOPER(next));
	    }
	    break;

	case LOGICAL:  /* modifier for EVAL and IFMATCH */
	    logical = scan->flags;
	    break;

/*******************************************************************

The CURLYX/WHILEM pair of ops handle the most generic case of the /A*B/
pattern, where A and B are subpatterns. (For simple A, CURLYM or
STAR/PLUS/CURLY/CURLYN are used instead.)

A*B is compiled as <CURLYX><A><WHILEM><B>

On entry to the subpattern, CURLYX is called. This pushes a CURLYX
state, which contains the current count, initialised to -1. It also sets
cur_curlyx to point to this state, with any previous value saved in the
state block.

CURLYX then jumps straight to the WHILEM op, rather than executing A,
since the pattern may possibly match zero times (i.e. it's a while {} loop
rather than a do {} while loop).

Each entry to WHILEM represents a successful match of A. The count in the
CURLYX block is incremented, another WHILEM state is pushed, and execution
passes to A or B depending on greediness and the current count.

For example, if matching against the string a1a2a3b (where the aN are
substrings that match /A/), then the match progresses as follows: (the
pushed states are interspersed with the bits of strings matched so far):

    <CURLYX cnt=-1>
    <CURLYX cnt=0><WHILEM>
    <CURLYX cnt=1><WHILEM> a1 <WHILEM>
    <CURLYX cnt=2><WHILEM> a1 <WHILEM> a2 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM> b

(Contrast this with something like CURLYM, which maintains only a single
backtrack state:

    <CURLYM cnt=0> a1
    a1 <CURLYM cnt=1> a2
    a1 a2 <CURLYM cnt=2> a3
    a1 a2 a3 <CURLYM cnt=3> b
)

Each WHILEM state block marks a point to backtrack to upon partial failure
of A or B, and also contains some minor state data related to that
iteration.  The CURLYX block, pointed to by cur_curlyx, contains the
overall state, such as the count, and pointers to the A and B ops.

This is complicated slightly by nested CURLYX/WHILEM's. Since cur_curlyx
must always point to the *current* CURLYX block, the rules are:

When executing CURLYX, save the old cur_curlyx in the CURLYX state block,
and set cur_curlyx to point the new block.

When popping the CURLYX block after a successful or unsuccessful match,
restore the previous cur_curlyx.

When WHILEM is about to execute B, save the current cur_curlyx, and set it
to the outer one saved in the CURLYX block.

When popping the WHILEM block after a successful or unsuccessful B match,
restore the previous cur_curlyx.

Here's an example for the pattern (AI* BI)*BO
I and O refer to inner and outer, C and W refer to CURLYX and WHILEM:

cur_
curlyx backtrack stack
------ ---------------
NULL   
CO     <CO prev=NULL> <WO>
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai 
CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi 
NULL   <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi <WO prev=CO> bo

At this point the pattern succeeds, and we work back down the stack to
clean up, restoring as we go:

CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi 
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai 
CO     <CO prev=NULL> <WO>
NULL   

*******************************************************************/

#define ST st->u.curlyx

	case CURLYX:    /* start of /A*B/  (for complex A) */
	{
	    /* No need to save/restore up to this paren */
	    I32 parenfloor = scan->flags;
	    
	    assert(next); /* keep Coverity happy */
	    if (OP(PREVOPER(next)) == NOTHING) /* LONGJMP */
		next += ARG(next);

	    /* XXXX Probably it is better to teach regpush to support
	       parenfloor > maxopenparen ... */
	    if (parenfloor > (I32)rex->lastparen)
		parenfloor = rex->lastparen; /* Pessimization... */

	    ST.prev_curlyx= cur_curlyx;
	    cur_curlyx = st;
	    ST.cp = PL_savestack_ix;

	    /* these fields contain the state of the current curly.
	     * they are accessed by subsequent WHILEMs */
	    ST.parenfloor = parenfloor;
	    ST.me = scan;
	    ST.B = next;
	    ST.minmod = minmod;
	    minmod = 0;
	    ST.count = -1;	/* this will be updated by WHILEM */
	    ST.lastloc = NULL;  /* this will be updated by WHILEM */

	    PUSH_YES_STATE_GOTO(CURLYX_end, PREVOPER(next), locinput);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */
	}

	case CURLYX_end: /* just finished matching all of A*B */
	    cur_curlyx = ST.prev_curlyx;
	    sayYES;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case CURLYX_end_fail: /* just failed to match all of A*B */
	    regcpblow(ST.cp);
	    cur_curlyx = ST.prev_curlyx;
	    sayNO;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */


#undef ST
#define ST st->u.whilem

	case WHILEM:     /* just matched an A in /A*B/  (for complex A) */
	{
	    /* see the discussion above about CURLYX/WHILEM */
	    I32 n;
	    int min, max;
	    regnode *A;

	    assert(cur_curlyx); /* keep Coverity happy */

	    min = ARG1(cur_curlyx->u.curlyx.me);
	    max = ARG2(cur_curlyx->u.curlyx.me);
	    A = NEXTOPER(cur_curlyx->u.curlyx.me) + EXTRA_STEP_2ARGS;
	    n = ++cur_curlyx->u.curlyx.count; /* how many A's matched */
	    ST.save_lastloc = cur_curlyx->u.curlyx.lastloc;
	    ST.cache_offset = 0;
	    ST.cache_mask = 0;
	    

	    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
		  "%*s  whilem: matched %ld out of %d..%d\n",
		  REPORT_CODE_OFF+depth*2, "", (long)n, min, max)
	    );

	    /* First just match a string of min A's. */

	    if (n < min) {
		ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor,
                                    maxopenparen);
		cur_curlyx->u.curlyx.lastloc = locinput;
		REGCP_SET(ST.lastcp);

		PUSH_STATE_GOTO(WHILEM_A_pre, A, locinput);
                /* NOTREACHED */
		NOT_REACHED; /* NOTREACHED */
	    }

	    /* If degenerate A matches "", assume A done. */

	    if (locinput == cur_curlyx->u.curlyx.lastloc) {
		DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
		   "%*s  whilem: empty match detected, trying continuation...\n",
		   REPORT_CODE_OFF+depth*2, "")
		);
		goto do_whilem_B_max;
	    }

	    /* super-linear cache processing.
             *
             * The idea here is that for certain types of CURLYX/WHILEM -
             * principally those whose upper bound is infinity (and
             * excluding regexes that have things like \1 and other very
             * non-regular expresssiony things), then if a pattern like
             * /....A*.../ fails and we backtrack to the WHILEM, then we
             * make a note that this particular WHILEM op was at string
             * position 47 (say) when the rest of pattern failed. Then, if
             * we ever find ourselves back at that WHILEM, and at string
             * position 47 again, we can just fail immediately rather than
             * running the rest of the pattern again.
             *
             * This is very handy when patterns start to go
             * 'super-linear', like in (a+)*(a+)*(a+)*, where you end up
             * with a combinatorial explosion of backtracking.
             *
             * The cache is implemented as a bit array, with one bit per
             * string byte position per WHILEM op (up to 16) - so its
             * between 0.25 and 2x the string size.
             *
             * To avoid allocating a poscache buffer every time, we do an
             * initially countdown; only after we have  executed a WHILEM
             * op (string-length x #WHILEMs) times do we allocate the
             * cache.
             *
             * The top 4 bits of scan->flags byte say how many different
             * relevant CURLLYX/WHILEM op pairs there are, while the
             * bottom 4-bits is the identifying index number of this
             * WHILEM.
             */

	    if (scan->flags) {

		if (!reginfo->poscache_maxiter) {
		    /* start the countdown: Postpone detection until we
		     * know the match is not *that* much linear. */
		    reginfo->poscache_maxiter
                        =    (reginfo->strend - reginfo->strbeg + 1)
                           * (scan->flags>>4);
		    /* possible overflow for long strings and many CURLYX's */
		    if (reginfo->poscache_maxiter < 0)
			reginfo->poscache_maxiter = I32_MAX;
		    reginfo->poscache_iter = reginfo->poscache_maxiter;
		}

		if (reginfo->poscache_iter-- == 0) {
		    /* initialise cache */
		    const SSize_t size = (reginfo->poscache_maxiter + 7)/8;
                    regmatch_info_aux *const aux = reginfo->info_aux;
		    if (aux->poscache) {
			if ((SSize_t)reginfo->poscache_size < size) {
			    Renew(aux->poscache, size, char);
			    reginfo->poscache_size = size;
			}
			Zero(aux->poscache, size, char);
		    }
		    else {
			reginfo->poscache_size = size;
			Newxz(aux->poscache, size, char);
		    }
		    DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
      "%swhilem: Detected a super-linear match, switching on caching%s...\n",
			      PL_colors[4], PL_colors[5])
		    );
		}

		if (reginfo->poscache_iter < 0) {
		    /* have we already failed at this position? */
		    SSize_t offset, mask;

                    reginfo->poscache_iter = -1; /* stop eventual underflow */
		    offset  = (scan->flags & 0xf) - 1
                                +   (locinput - reginfo->strbeg)
                                  * (scan->flags>>4);
		    mask    = 1 << (offset % 8);
		    offset /= 8;
		    if (reginfo->info_aux->poscache[offset] & mask) {
			DEBUG_EXECUTE_r( PerlIO_printf(Perl_debug_log,
			    "%*s  whilem: (cache) already tried at this position...\n",
			    REPORT_CODE_OFF+depth*2, "")
			);
			sayNO; /* cache records failure */
		    }
		    ST.cache_offset = offset;
		    ST.cache_mask   = mask;
		}
	    }

	    /* Prefer B over A for minimal matching. */

	    if (cur_curlyx->u.curlyx.minmod) {
		ST.save_curlyx = cur_curlyx;
		cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
		ST.cp = regcppush(rex, ST.save_curlyx->u.curlyx.parenfloor,
                            maxopenparen);
		REGCP_SET(ST.lastcp);
		PUSH_YES_STATE_GOTO(WHILEM_B_min, ST.save_curlyx->u.curlyx.B,
                                    locinput);
                /* NOTREACHED */
		NOT_REACHED; /* NOTREACHED */
	    }

	    /* Prefer A over B for maximal matching. */

	    if (n < max) { /* More greed allowed? */
		ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor,
                            maxopenparen);
		cur_curlyx->u.curlyx.lastloc = locinput;
		REGCP_SET(ST.lastcp);
		PUSH_STATE_GOTO(WHILEM_A_max, A, locinput);
                /* NOTREACHED */
		NOT_REACHED; /* NOTREACHED */
	    }
	    goto do_whilem_B_max;
	}
        /* NOTREACHED */
	NOT_REACHED; /* NOTREACHED */

	case WHILEM_B_min: /* just matched B in a minimal match */
	case WHILEM_B_max: /* just matched B in a maximal match */
	    cur_curlyx = ST.save_curlyx;
	    sayYES;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case WHILEM_B_max_fail: /* just failed to match B in a maximal match */
	    cur_curlyx = ST.save_curlyx;
	    cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
	    cur_curlyx->u.curlyx.count--;
	    CACHEsayNO;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case WHILEM_A_min_fail: /* just failed to match A in a minimal match */
	    /* FALLTHROUGH */
	case WHILEM_A_pre_fail: /* just failed to match even minimal A */
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex, &maxopenparen);
	    cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
	    cur_curlyx->u.curlyx.count--;
	    CACHEsayNO;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case WHILEM_A_max_fail: /* just failed to match A in a maximal match */
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex, &maxopenparen); /* Restore some previous $<digit>s? */
	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		"%*s  whilem: failed, trying continuation...\n",
		REPORT_CODE_OFF+depth*2, "")
	    );
	  do_whilem_B_max:
	    if (cur_curlyx->u.curlyx.count >= REG_INFTY
		&& ckWARN(WARN_REGEXP)
		&& !reginfo->warned)
	    {
                reginfo->warned	= TRUE;
		Perl_warner(aTHX_ packWARN(WARN_REGEXP),
		     "Complex regular subexpression recursion limit (%d) "
		     "exceeded",
		     REG_INFTY - 1);
	    }

	    /* now try B */
	    ST.save_curlyx = cur_curlyx;
	    cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
	    PUSH_YES_STATE_GOTO(WHILEM_B_max, ST.save_curlyx->u.curlyx.B,
                                locinput);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case WHILEM_B_min_fail: /* just failed to match B in a minimal match */
	    cur_curlyx = ST.save_curlyx;
	    REGCP_UNWIND(ST.lastcp);
	    regcppop(rex, &maxopenparen);

	    if (cur_curlyx->u.curlyx.count >= /*max*/ARG2(cur_curlyx->u.curlyx.me)) {
		/* Maximum greed exceeded */
		if (cur_curlyx->u.curlyx.count >= REG_INFTY
		    && ckWARN(WARN_REGEXP)
                    && !reginfo->warned)
		{
                    reginfo->warned	= TRUE;
		    Perl_warner(aTHX_ packWARN(WARN_REGEXP),
			"Complex regular subexpression recursion "
			"limit (%d) exceeded",
			REG_INFTY - 1);
		}
		cur_curlyx->u.curlyx.count--;
		CACHEsayNO;
	    }

	    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
		"%*s  trying longer...\n", REPORT_CODE_OFF+depth*2, "")
	    );
	    /* Try grabbing another A and see if it helps. */
	    cur_curlyx->u.curlyx.lastloc = locinput;
	    ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor,
                            maxopenparen);
	    REGCP_SET(ST.lastcp);
	    PUSH_STATE_GOTO(WHILEM_A_min,
		/*A*/ NEXTOPER(ST.save_curlyx->u.curlyx.me) + EXTRA_STEP_2ARGS,
                locinput);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

#undef  ST
#define ST st->u.branch

	case BRANCHJ:	    /*  /(...|A|...)/ with long next pointer */
	    next = scan + ARG(scan);
	    if (next == scan)
		next = NULL;
	    scan = NEXTOPER(scan);
	    /* FALLTHROUGH */

	case BRANCH:	    /*  /(...|A|...)/ */
	    scan = NEXTOPER(scan); /* scan now points to inner node */
	    ST.lastparen = rex->lastparen;
	    ST.lastcloseparen = rex->lastcloseparen;
	    ST.next_branch = next;
	    REGCP_SET(ST.cp);

	    /* Now go into the branch */
	    if (has_cutgroup) {
	        PUSH_YES_STATE_GOTO(BRANCH_next, scan, locinput);
	    } else {
	        PUSH_STATE_GOTO(BRANCH_next, scan, locinput);
	    }
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

        case CUTGROUP:  /*  /(*THEN)/  */
            sv_yes_mark = st->u.mark.mark_name = scan->flags ? NULL :
                MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
            PUSH_STATE_GOTO(CUTGROUP_next, next, locinput);
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

        case CUTGROUP_next_fail:
            do_cutgroup = 1;
            no_final = 1;
            if (st->u.mark.mark_name)
                sv_commit = st->u.mark.mark_name;
            sayNO;	    
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

        case BRANCH_next:
            sayYES;
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

	case BRANCH_next_fail: /* that branch failed; try the next, if any */
	    if (do_cutgroup) {
	        do_cutgroup = 0;
	        no_final = 0;
	    }
	    REGCP_UNWIND(ST.cp);
            UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
	    scan = ST.next_branch;
	    /* no more branches? */
	    if (!scan || (OP(scan) != BRANCH && OP(scan) != BRANCHJ)) {
	        DEBUG_EXECUTE_r({
		    PerlIO_printf( Perl_debug_log,
			"%*s  %sBRANCH failed...%s\n",
			REPORT_CODE_OFF+depth*2, "", 
			PL_colors[4],
			PL_colors[5] );
		});
		sayNO_SILENT;
            }
	    continue; /* execute next BRANCH[J] op */
            /* NOTREACHED */
    
	case MINMOD: /* next op will be non-greedy, e.g. A*?  */
	    minmod = 1;
	    break;

#undef  ST
#define ST st->u.curlym

	case CURLYM:	/* /A{m,n}B/ where A is fixed-length */

	    /* This is an optimisation of CURLYX that enables us to push
	     * only a single backtracking state, no matter how many matches
	     * there are in {m,n}. It relies on the pattern being constant
	     * length, with no parens to influence future backrefs
	     */

	    ST.me = scan;
	    scan = NEXTOPER(scan) + NODE_STEP_REGNODE;

	    ST.lastparen      = rex->lastparen;
	    ST.lastcloseparen = rex->lastcloseparen;

	    /* if paren positive, emulate an OPEN/CLOSE around A */
	    if (ST.me->flags) {
		U32 paren = ST.me->flags;
		if (paren > maxopenparen)
		    maxopenparen = paren;
		scan += NEXT_OFF(scan); /* Skip former OPEN. */
	    }
	    ST.A = scan;
	    ST.B = next;
	    ST.alen = 0;
	    ST.count = 0;
	    ST.minmod = minmod;
	    minmod = 0;
	    ST.c1 = CHRTEST_UNINIT;
	    REGCP_SET(ST.cp);

	    if (!(ST.minmod ? ARG1(ST.me) : ARG2(ST.me))) /* min/max */
		goto curlym_do_B;

	  curlym_do_A: /* execute the A in /A{m,n}B/  */
	    PUSH_YES_STATE_GOTO(CURLYM_A, ST.A, locinput); /* match A */
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case CURLYM_A: /* we've just matched an A */
	    ST.count++;
	    /* after first match, determine A's length: u.curlym.alen */
	    if (ST.count == 1) {
		if (reginfo->is_utf8_target) {
		    char *s = st->locinput;
		    while (s < locinput) {
			ST.alen++;
			s += UTF8SKIP(s);
		    }
		}
		else {
		    ST.alen = locinput - st->locinput;
		}
		if (ST.alen == 0)
		    ST.count = ST.minmod ? ARG1(ST.me) : ARG2(ST.me);
	    }
	    DEBUG_EXECUTE_r(
		PerlIO_printf(Perl_debug_log,
			  "%*s  CURLYM now matched %"IVdf" times, len=%"IVdf"...\n",
			  (int)(REPORT_CODE_OFF+(depth*2)), "",
			  (IV) ST.count, (IV)ST.alen)
	    );

	    if (cur_eval && cur_eval->u.eval.close_paren && 
	        cur_eval->u.eval.close_paren == (U32)ST.me->flags) 
	        goto fake_end;
	        
	    {
		I32 max = (ST.minmod ? ARG1(ST.me) : ARG2(ST.me));
		if ( max == REG_INFTY || ST.count < max )
		    goto curlym_do_A; /* try to match another A */
	    }
	    goto curlym_do_B; /* try to match B */

	case CURLYM_A_fail: /* just failed to match an A */
	    REGCP_UNWIND(ST.cp);

	    if (ST.minmod || ST.count < ARG1(ST.me) /* min*/ 
	        || (cur_eval && cur_eval->u.eval.close_paren &&
	            cur_eval->u.eval.close_paren == (U32)ST.me->flags))
		sayNO;

	  curlym_do_B: /* execute the B in /A{m,n}B/  */
	    if (ST.c1 == CHRTEST_UNINIT) {
		/* calculate c1 and c2 for possible match of 1st char
		 * following curly */
		ST.c1 = ST.c2 = CHRTEST_VOID;
                assert(ST.B);
		if (HAS_TEXT(ST.B) || JUMPABLE(ST.B)) {
		    regnode *text_node = ST.B;
		    if (! HAS_TEXT(text_node))
			FIND_NEXT_IMPT(text_node);
	            /* this used to be 
	                
	                (HAS_TEXT(text_node) && PL_regkind[OP(text_node)] == EXACT)
	                
	            	But the former is redundant in light of the latter.
	            	
	            	if this changes back then the macro for 
	            	IS_TEXT and friends need to change.
	             */
		    if (PL_regkind[OP(text_node)] == EXACT) {
                        if (! S_setup_EXACTISH_ST_c1_c2(aTHX_
                           text_node, &ST.c1, ST.c1_utf8, &ST.c2, ST.c2_utf8,
                           reginfo))
                        {
                            sayNO;
                        }
		    }
		}
	    }

	    DEBUG_EXECUTE_r(
		PerlIO_printf(Perl_debug_log,
		    "%*s  CURLYM trying tail with matches=%"IVdf"...\n",
		    (int)(REPORT_CODE_OFF+(depth*2)),
		    "", (IV)ST.count)
		);
	    if (! NEXTCHR_IS_EOS && ST.c1 != CHRTEST_VOID) {
                if (! UTF8_IS_INVARIANT(nextchr) && utf8_target) {
                    if (memNE(locinput, ST.c1_utf8, UTF8SKIP(locinput))
                        && memNE(locinput, ST.c2_utf8, UTF8SKIP(locinput)))
                    {
                        /* simulate B failing */
                        DEBUG_OPTIMISE_r(
                            PerlIO_printf(Perl_debug_log,
                                "%*s  CURLYM Fast bail next target=0x%"UVXf" c1=0x%"UVXf" c2=0x%"UVXf"\n",
                                (int)(REPORT_CODE_OFF+(depth*2)),"",
                                valid_utf8_to_uvchr((U8 *) locinput, NULL),
                                valid_utf8_to_uvchr(ST.c1_utf8, NULL),
                                valid_utf8_to_uvchr(ST.c2_utf8, NULL))
                        );
                        state_num = CURLYM_B_fail;
                        goto reenter_switch;
                    }
                }
                else if (nextchr != ST.c1 && nextchr != ST.c2) {
                    /* simulate B failing */
                    DEBUG_OPTIMISE_r(
                        PerlIO_printf(Perl_debug_log,
                            "%*s  CURLYM Fast bail next target=0x%X c1=0x%X c2=0x%X\n",
                            (int)(REPORT_CODE_OFF+(depth*2)),"",
                            (int) nextchr, ST.c1, ST.c2)
                    );
                    state_num = CURLYM_B_fail;
                    goto reenter_switch;
                }
            }

	    if (ST.me->flags) {
		/* emulate CLOSE: mark current A as captured */
		I32 paren = ST.me->flags;
		if (ST.count) {
		    rex->offs[paren].start
			= HOPc(locinput, -ST.alen) - reginfo->strbeg;
		    rex->offs[paren].end = locinput - reginfo->strbeg;
		    if ((U32)paren > rex->lastparen)
			rex->lastparen = paren;
		    rex->lastcloseparen = paren;
		}
		else
		    rex->offs[paren].end = -1;
		if (cur_eval && cur_eval->u.eval.close_paren &&
		    cur_eval->u.eval.close_paren == (U32)ST.me->flags) 
		{
		    if (ST.count) 
	                goto fake_end;
	            else
	                sayNO;
	        }
	    }
	    
	    PUSH_STATE_GOTO(CURLYM_B, ST.B, locinput); /* match B */
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case CURLYM_B_fail: /* just failed to match a B */
	    REGCP_UNWIND(ST.cp);
            UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
	    if (ST.minmod) {
		I32 max = ARG2(ST.me);
		if (max != REG_INFTY && ST.count == max)
		    sayNO;
		goto curlym_do_A; /* try to match a further A */
	    }
	    /* backtrack one A */
	    if (ST.count == ARG1(ST.me) /* min */)
		sayNO;
	    ST.count--;
	    SET_locinput(HOPc(locinput, -ST.alen));
	    goto curlym_do_B; /* try to match B */

#undef ST
#define ST st->u.curly

#define CURLY_SETPAREN(paren, success) \
    if (paren) { \
	if (success) { \
	    rex->offs[paren].start = HOPc(locinput, -1) - reginfo->strbeg; \
	    rex->offs[paren].end = locinput - reginfo->strbeg; \
	    if (paren > rex->lastparen) \
		rex->lastparen = paren; \
	    rex->lastcloseparen = paren; \
	} \
	else { \
	    rex->offs[paren].end = -1; \
	    rex->lastparen      = ST.lastparen; \
	    rex->lastcloseparen = ST.lastcloseparen; \
	} \
    }

        case STAR:		/*  /A*B/ where A is width 1 char */
	    ST.paren = 0;
	    ST.min = 0;
	    ST.max = REG_INFTY;
	    scan = NEXTOPER(scan);
	    goto repeat;

        case PLUS:		/*  /A+B/ where A is width 1 char */
	    ST.paren = 0;
	    ST.min = 1;
	    ST.max = REG_INFTY;
	    scan = NEXTOPER(scan);
	    goto repeat;

	case CURLYN:		/*  /(A){m,n}B/ where A is width 1 char */
            ST.paren = scan->flags;	/* Which paren to set */
            ST.lastparen      = rex->lastparen;
	    ST.lastcloseparen = rex->lastcloseparen;
	    if (ST.paren > maxopenparen)
		maxopenparen = ST.paren;
	    ST.min = ARG1(scan);  /* min to match */
	    ST.max = ARG2(scan);  /* max to match */
	    if (cur_eval && cur_eval->u.eval.close_paren &&
	        cur_eval->u.eval.close_paren == (U32)ST.paren) {
	        ST.min=1;
	        ST.max=1;
	    }
            scan = regnext(NEXTOPER(scan) + NODE_STEP_REGNODE);
	    goto repeat;

	case CURLY:		/*  /A{m,n}B/ where A is width 1 char */
	    ST.paren = 0;
	    ST.min = ARG1(scan);  /* min to match */
	    ST.max = ARG2(scan);  /* max to match */
	    scan = NEXTOPER(scan) + NODE_STEP_REGNODE;
	  repeat:
	    /*
	    * Lookahead to avoid useless match attempts
	    * when we know what character comes next.
	    *
	    * Used to only do .*x and .*?x, but now it allows
	    * for )'s, ('s and (?{ ... })'s to be in the way
	    * of the quantifier and the EXACT-like node.  -- japhy
	    */

	    assert(ST.min <= ST.max);
            if (! HAS_TEXT(next) && ! JUMPABLE(next)) {
                ST.c1 = ST.c2 = CHRTEST_VOID;
            }
            else {
		regnode *text_node = next;

		if (! HAS_TEXT(text_node)) 
		    FIND_NEXT_IMPT(text_node);

		if (! HAS_TEXT(text_node))
		    ST.c1 = ST.c2 = CHRTEST_VOID;
		else {
		    if ( PL_regkind[OP(text_node)] != EXACT ) {
			ST.c1 = ST.c2 = CHRTEST_VOID;
		    }
		    else {
                    
                    /*  Currently we only get here when 
                        
                        PL_rekind[OP(text_node)] == EXACT
                    
                        if this changes back then the macro for IS_TEXT and 
                        friends need to change. */
                        if (! S_setup_EXACTISH_ST_c1_c2(aTHX_
                           text_node, &ST.c1, ST.c1_utf8, &ST.c2, ST.c2_utf8,
                           reginfo))
                        {
                            sayNO;
                        }
                    }
		}
	    }

	    ST.A = scan;
	    ST.B = next;
	    if (minmod) {
                char *li = locinput;
		minmod = 0;
		if (ST.min &&
                        regrepeat(rex, &li, ST.A, reginfo, ST.min, depth)
                            < ST.min)
		    sayNO;
                SET_locinput(li);
		ST.count = ST.min;
		REGCP_SET(ST.cp);
		if (ST.c1 == CHRTEST_VOID)
		    goto curly_try_B_min;

		ST.oldloc = locinput;

		/* set ST.maxpos to the furthest point along the
		 * string that could possibly match */
		if  (ST.max == REG_INFTY) {
		    ST.maxpos = reginfo->strend - 1;
		    if (utf8_target)
			while (UTF8_IS_CONTINUATION(*(U8*)ST.maxpos))
			    ST.maxpos--;
		}
		else if (utf8_target) {
		    int m = ST.max - ST.min;
		    for (ST.maxpos = locinput;
			 m >0 && ST.maxpos < reginfo->strend; m--)
			ST.maxpos += UTF8SKIP(ST.maxpos);
		}
		else {
		    ST.maxpos = locinput + ST.max - ST.min;
		    if (ST.maxpos >= reginfo->strend)
			ST.maxpos = reginfo->strend - 1;
		}
		goto curly_try_B_min_known;

	    }
	    else {
                /* avoid taking address of locinput, so it can remain
                 * a register var */
                char *li = locinput;
		ST.count = regrepeat(rex, &li, ST.A, reginfo, ST.max, depth);
		if (ST.count < ST.min)
		    sayNO;
                SET_locinput(li);
		if ((ST.count > ST.min)
		    && (PL_regkind[OP(ST.B)] == EOL) && (OP(ST.B) != MEOL))
		{
		    /* A{m,n} must come at the end of the string, there's
		     * no point in backing off ... */
		    ST.min = ST.count;
		    /* ...except that $ and \Z can match before *and* after
		       newline at the end.  Consider "\n\n" =~ /\n+\Z\n/.
		       We may back off by one in this case. */
		    if (UCHARAT(locinput - 1) == '\n' && OP(ST.B) != EOS)
			ST.min--;
		}
		REGCP_SET(ST.cp);
		goto curly_try_B_max;
	    }
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case CURLY_B_min_known_fail:
	    /* failed to find B in a non-greedy match where c1,c2 valid */

	    REGCP_UNWIND(ST.cp);
            if (ST.paren) {
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            }
	    /* Couldn't or didn't -- move forward. */
	    ST.oldloc = locinput;
	    if (utf8_target)
		locinput += UTF8SKIP(locinput);
	    else
		locinput++;
	    ST.count++;
	  curly_try_B_min_known:
	     /* find the next place where 'B' could work, then call B */
	    {
		int n;
		if (utf8_target) {
		    n = (ST.oldloc == locinput) ? 0 : 1;
		    if (ST.c1 == ST.c2) {
			/* set n to utf8_distance(oldloc, locinput) */
			while (locinput <= ST.maxpos
                              && memNE(locinput, ST.c1_utf8, UTF8SKIP(locinput)))
                        {
			    locinput += UTF8SKIP(locinput);
			    n++;
			}
		    }
		    else {
			/* set n to utf8_distance(oldloc, locinput) */
			while (locinput <= ST.maxpos
                              && memNE(locinput, ST.c1_utf8, UTF8SKIP(locinput))
                              && memNE(locinput, ST.c2_utf8, UTF8SKIP(locinput)))
                        {
			    locinput += UTF8SKIP(locinput);
			    n++;
			}
		    }
		}
		else {  /* Not utf8_target */
		    if (ST.c1 == ST.c2) {
			while (locinput <= ST.maxpos &&
			       UCHARAT(locinput) != ST.c1)
			    locinput++;
		    }
		    else {
			while (locinput <= ST.maxpos
			       && UCHARAT(locinput) != ST.c1
			       && UCHARAT(locinput) != ST.c2)
			    locinput++;
		    }
		    n = locinput - ST.oldloc;
		}
		if (locinput > ST.maxpos)
		    sayNO;
		if (n) {
                    /* In /a{m,n}b/, ST.oldloc is at "a" x m, locinput is
                     * at b; check that everything between oldloc and
                     * locinput matches */
                    char *li = ST.oldloc;
		    ST.count += n;
		    if (regrepeat(rex, &li, ST.A, reginfo, n, depth) < n)
			sayNO;
                    assert(n == REG_INFTY || locinput == li);
		}
		CURLY_SETPAREN(ST.paren, ST.count);
		if (cur_eval && cur_eval->u.eval.close_paren && 
		    cur_eval->u.eval.close_paren == (U32)ST.paren) {
		    goto fake_end;
	        }
		PUSH_STATE_GOTO(CURLY_B_min_known, ST.B, locinput);
	    }
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case CURLY_B_min_fail:
	    /* failed to find B in a non-greedy match where c1,c2 invalid */

	    REGCP_UNWIND(ST.cp);
            if (ST.paren) {
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            }
	    /* failed -- move forward one */
            {
                char *li = locinput;
                if (!regrepeat(rex, &li, ST.A, reginfo, 1, depth)) {
                    sayNO;
                }
                locinput = li;
            }
            {
		ST.count++;
		if (ST.count <= ST.max || (ST.max == REG_INFTY &&
			ST.count > 0)) /* count overflow ? */
		{
		  curly_try_B_min:
		    CURLY_SETPAREN(ST.paren, ST.count);
		    if (cur_eval && cur_eval->u.eval.close_paren &&
		        cur_eval->u.eval.close_paren == (U32)ST.paren) {
                        goto fake_end;
                    }
		    PUSH_STATE_GOTO(CURLY_B_min, ST.B, locinput);
		}
	    }
            sayNO;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

          curly_try_B_max:
	    /* a successful greedy match: now try to match B */
            if (cur_eval && cur_eval->u.eval.close_paren &&
                cur_eval->u.eval.close_paren == (U32)ST.paren) {
                goto fake_end;
            }
	    {
		bool could_match = locinput < reginfo->strend;

		/* If it could work, try it. */
                if (ST.c1 != CHRTEST_VOID && could_match) {
                    if (! UTF8_IS_INVARIANT(UCHARAT(locinput)) && utf8_target)
                    {
                        could_match = memEQ(locinput,
                                            ST.c1_utf8,
                                            UTF8SKIP(locinput))
                                    || memEQ(locinput,
                                             ST.c2_utf8,
                                             UTF8SKIP(locinput));
                    }
                    else {
                        could_match = UCHARAT(locinput) == ST.c1
                                      || UCHARAT(locinput) == ST.c2;
                    }
                }
                if (ST.c1 == CHRTEST_VOID || could_match) {
		    CURLY_SETPAREN(ST.paren, ST.count);
		    PUSH_STATE_GOTO(CURLY_B_max, ST.B, locinput);
                    /* NOTREACHED */
		    NOT_REACHED; /* NOTREACHED */
		}
	    }
	    /* FALLTHROUGH */

	case CURLY_B_max_fail:
	    /* failed to find B in a greedy match */

	    REGCP_UNWIND(ST.cp);
            if (ST.paren) {
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            }
	    /*  back up. */
	    if (--ST.count < ST.min)
		sayNO;
	    locinput = HOPc(locinput, -1);
	    goto curly_try_B_max;

#undef ST

	case END: /*  last op of main pattern  */
          fake_end:
	    if (cur_eval) {
		/* we've just finished A in /(??{A})B/; now continue with B */

		st->u.eval.prev_rex = rex_sv;		/* inner */

                /* Save *all* the positions. */
		st->u.eval.cp = regcppush(rex, 0, maxopenparen);
		rex_sv = cur_eval->u.eval.prev_rex;
		is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
		SET_reg_curpm(rex_sv);
		rex = ReANY(rex_sv);
		rexi = RXi_GET(rex);
		cur_curlyx = cur_eval->u.eval.prev_curlyx;

		REGCP_SET(st->u.eval.lastcp);

		/* Restore parens of the outer rex without popping the
		 * savestack */
		S_regcp_restore(aTHX_ rex, cur_eval->u.eval.lastcp,
                                        &maxopenparen);

		st->u.eval.prev_eval = cur_eval;
		cur_eval = cur_eval->u.eval.prev_eval;
		DEBUG_EXECUTE_r(
		    PerlIO_printf(Perl_debug_log, "%*s  EVAL trying tail ... %"UVxf"\n",
				      REPORT_CODE_OFF+depth*2, "",PTR2UV(cur_eval)););
                if ( nochange_depth )
	            nochange_depth--;

                PUSH_YES_STATE_GOTO(EVAL_AB, st->u.eval.prev_eval->u.eval.B,
                                    locinput); /* match B */
	    }

	    if (locinput < reginfo->till) {
		DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
				      "%sMatch possible, but length=%ld is smaller than requested=%ld, failing!%s\n",
				      PL_colors[4],
				      (long)(locinput - startpos),
				      (long)(reginfo->till - startpos),
				      PL_colors[5]));
               				      
		sayNO_SILENT;		/* Cannot match: too short. */
	    }
	    sayYES;			/* Success! */

	case SUCCEED: /* successful SUSPEND/UNLESSM/IFMATCH/CURLYM */
	    DEBUG_EXECUTE_r(
	    PerlIO_printf(Perl_debug_log,
		"%*s  %ssubpattern success...%s\n",
		REPORT_CODE_OFF+depth*2, "", PL_colors[4], PL_colors[5]));
	    sayYES;			/* Success! */

#undef  ST
#define ST st->u.ifmatch

        {
            char *newstart;

	case SUSPEND:	/* (?>A) */
	    ST.wanted = 1;
	    newstart = locinput;
	    goto do_ifmatch;	

	case UNLESSM:	/* -ve lookaround: (?!A), or with flags, (?<!A) */
	    ST.wanted = 0;
	    goto ifmatch_trivial_fail_test;

	case IFMATCH:	/* +ve lookaround: (?=A), or with flags, (?<=A) */
	    ST.wanted = 1;
	  ifmatch_trivial_fail_test:
	    if (scan->flags) {
		char * const s = HOPBACKc(locinput, scan->flags);
		if (!s) {
		    /* trivial fail */
		    if (logical) {
			logical = 0;
			sw = 1 - cBOOL(ST.wanted);
		    }
		    else if (ST.wanted)
			sayNO;
		    next = scan + ARG(scan);
		    if (next == scan)
			next = NULL;
		    break;
		}
		newstart = s;
	    }
	    else
		newstart = locinput;

	  do_ifmatch:
	    ST.me = scan;
	    ST.logical = logical;
	    logical = 0; /* XXX: reset state of logical once it has been saved into ST */
	    
	    /* execute body of (?...A) */
	    PUSH_YES_STATE_GOTO(IFMATCH_A, NEXTOPER(NEXTOPER(scan)), newstart);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */
        }

	case IFMATCH_A_fail: /* body of (?...A) failed */
	    ST.wanted = !ST.wanted;
	    /* FALLTHROUGH */

	case IFMATCH_A: /* body of (?...A) succeeded */
	    if (ST.logical) {
		sw = cBOOL(ST.wanted);
	    }
	    else if (!ST.wanted)
		sayNO;

	    if (OP(ST.me) != SUSPEND) {
                /* restore old position except for (?>...) */
		locinput = st->locinput;
	    }
	    scan = ST.me + ARG(ST.me);
	    if (scan == ST.me)
		scan = NULL;
	    continue; /* execute B */

#undef ST

	case LONGJMP: /*  alternative with many branches compiles to
                       * (BRANCHJ; EXACT ...; LONGJMP ) x N */
	    next = scan + ARG(scan);
	    if (next == scan)
		next = NULL;
	    break;

	case COMMIT:  /*  (*COMMIT)  */
	    reginfo->cutpoint = reginfo->strend;
	    /* FALLTHROUGH */

	case PRUNE:   /*  (*PRUNE)   */
	    if (!scan->flags)
	        sv_yes_mark = sv_commit = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
	    PUSH_STATE_GOTO(COMMIT_next, next, locinput);
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

	case COMMIT_next_fail:
	    no_final = 1;    
	    /* FALLTHROUGH */	    

	case OPFAIL:   /* (*FAIL)  */
	    sayNO;
            /* NOTREACHED */
	    NOT_REACHED; /* NOTREACHED */

#define ST st->u.mark
        case MARKPOINT: /*  (*MARK:foo)  */
            ST.prev_mark = mark_state;
            ST.mark_name = sv_commit = sv_yes_mark 
                = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
            mark_state = st;
            ST.mark_loc = locinput;
            PUSH_YES_STATE_GOTO(MARKPOINT_next, next, locinput);
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

        case MARKPOINT_next:
            mark_state = ST.prev_mark;
            sayYES;
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

        case MARKPOINT_next_fail:
            if (popmark && sv_eq(ST.mark_name,popmark)) 
            {
                if (ST.mark_loc > startpoint)
	            reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
                popmark = NULL; /* we found our mark */
                sv_commit = ST.mark_name;

                DEBUG_EXECUTE_r({
                        PerlIO_printf(Perl_debug_log,
		            "%*s  %ssetting cutpoint to mark:%"SVf"...%s\n",
		            REPORT_CODE_OFF+depth*2, "", 
		            PL_colors[4], SVfARG(sv_commit), PL_colors[5]);
		});
            }
            mark_state = ST.prev_mark;
            sv_yes_mark = mark_state ? 
                mark_state->u.mark.mark_name : NULL;
            sayNO;
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */

        case SKIP:  /*  (*SKIP)  */
            if (scan->flags) {
                /* (*SKIP) : if we fail we cut here*/
                ST.mark_name = NULL;
                ST.mark_loc = locinput;
                PUSH_STATE_GOTO(SKIP_next,next, locinput);
            } else {
                /* (*SKIP:NAME) : if there is a (*MARK:NAME) fail where it was, 
                   otherwise do nothing.  Meaning we need to scan 
                 */
                regmatch_state *cur = mark_state;
                SV *find = MUTABLE_SV(rexi->data->data[ ARG( scan ) ]);
                
                while (cur) {
                    if ( sv_eq( cur->u.mark.mark_name, 
                                find ) ) 
                    {
                        ST.mark_name = find;
                        PUSH_STATE_GOTO( SKIP_next, next, locinput);
                    }
                    cur = cur->u.mark.prev_mark;
                }
            }    
            /* Didn't find our (*MARK:NAME) so ignore this (*SKIP:NAME) */
            break;    

	case SKIP_next_fail:
	    if (ST.mark_name) {
	        /* (*CUT:NAME) - Set up to search for the name as we 
	           collapse the stack*/
	        popmark = ST.mark_name;	   
	    } else {
	        /* (*CUT) - No name, we cut here.*/
	        if (ST.mark_loc > startpoint)
	            reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
	        /* but we set sv_commit to latest mark_name if there
	           is one so they can test to see how things lead to this
	           cut */    
                if (mark_state) 
                    sv_commit=mark_state->u.mark.mark_name;	            
            } 
            no_final = 1; 
            sayNO;
            /* NOTREACHED */
            NOT_REACHED; /* NOTREACHED */
#undef ST

        case LNBREAK: /* \R */
            if ((n=is_LNBREAK_safe(locinput, reginfo->strend, utf8_target))) {
                locinput += n;
            } else
                sayNO;
            break;

	default:
	    PerlIO_printf(Perl_error_log, "%"UVxf" %d\n",
			  PTR2UV(scan), OP(scan));
	    Perl_croak(aTHX_ "regexp memory corruption");

        /* this is a point to jump to in order to increment
         * locinput by one character */
          increment_locinput:
            assert(!NEXTCHR_IS_EOS);
            if (utf8_target) {
                locinput += PL_utf8skip[nextchr];
                /* locinput is allowed to go 1 char off the end, but not 2+ */
                if (locinput > reginfo->strend)
                    sayNO;
            }
            else
                locinput++;
            break;
	    
	} /* end switch */ 

        /* switch break jumps here */
	scan = next; /* prepare to execute the next op and ... */
	continue;    /* ... jump back to the top, reusing st */
        /* NOTREACHED */

      push_yes_state:
	/* push a state that backtracks on success */
	st->u.yes.prev_yes_state = yes_state;
	yes_state = st;
	/* FALLTHROUGH */
      push_state:
	/* push a new regex state, then continue at scan  */
	{
	    regmatch_state *newst;

	    DEBUG_STACK_r({
	        regmatch_state *cur = st;
	        regmatch_state *curyes = yes_state;
	        int curd = depth;
	        regmatch_slab *slab = PL_regmatch_slab;
                for (;curd > -1;cur--,curd--) {
                    if (cur < SLAB_FIRST(slab)) {
                	slab = slab->prev;
                	cur = SLAB_LAST(slab);
                    }
                    PerlIO_printf(Perl_error_log, "%*s#%-3d %-10s %s\n",
                        REPORT_CODE_OFF + 2 + depth * 2,"",
                        curd, PL_reg_name[cur->resume_state],
                        (curyes == cur) ? "yes" : ""
                    );
                    if (curyes == cur)
	                curyes = cur->u.yes.prev_yes_state;
                }
            } else 
                DEBUG_STATE_pp("push")
            );
	    depth++;
	    st->locinput = locinput;
	    newst = st+1; 
	    if (newst >  SLAB_LAST(PL_regmatch_slab))
		newst = S_push_slab(aTHX);
	    PL_regmatch_state = newst;

	    locinput = pushinput;
	    st = newst;
	    continue;
            /* NOTREACHED */
	}
    }

    /*
    * We get here only if there's trouble -- normally "case END" is
    * the terminating point.
    */
    Perl_croak(aTHX_ "corrupted regexp pointers");
    /* NOTREACHED */
    sayNO;
    NOT_REACHED; /* NOTREACHED */

  yes:
    if (yes_state) {
	/* we have successfully completed a subexpression, but we must now
	 * pop to the state marked by yes_state and continue from there */
	assert(st != yes_state);
#ifdef DEBUGGING
	while (st != yes_state) {
	    st--;
	    if (st < SLAB_FIRST(PL_regmatch_slab)) {
		PL_regmatch_slab = PL_regmatch_slab->prev;
		st = SLAB_LAST(PL_regmatch_slab);
	    }
	    DEBUG_STATE_r({
	        if (no_final) {
	            DEBUG_STATE_pp("pop (no final)");        
	        } else {
	            DEBUG_STATE_pp("pop (yes)");
	        }
	    });
	    depth--;
	}
#else
	while (yes_state < SLAB_FIRST(PL_regmatch_slab)
	    || yes_state > SLAB_LAST(PL_regmatch_slab))
	{
	    /* not in this slab, pop slab */
	    depth -= (st - SLAB_FIRST(PL_regmatch_slab) + 1);
	    PL_regmatch_slab = PL_regmatch_slab->prev;
	    st = SLAB_LAST(PL_regmatch_slab);
	}
	depth -= (st - yes_state);
#endif
	st = yes_state;
	yes_state = st->u.yes.prev_yes_state;
	PL_regmatch_state = st;
        
        if (no_final)
            locinput= st->locinput;
	state_num = st->resume_state + no_final;
	goto reenter_switch;
    }

    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%sMatch successful!%s\n",
			  PL_colors[4], PL_colors[5]));

    if (reginfo->info_aux_eval) {
	/* each successfully executed (?{...}) block does the equivalent of
	 *   local $^R = do {...}
	 * When popping the save stack, all these locals would be undone;
	 * bypass this by setting the outermost saved $^R to the latest
	 * value */
        /* I dont know if this is needed or works properly now.
         * see code related to PL_replgv elsewhere in this file.
         * Yves
         */
	if (oreplsv != GvSV(PL_replgv))
	    sv_setsv(oreplsv, GvSV(PL_replgv));
    }
    result = 1;
    goto final_exit;

  no:
    DEBUG_EXECUTE_r(
	PerlIO_printf(Perl_debug_log,
            "%*s  %sfailed...%s\n",
            REPORT_CODE_OFF+depth*2, "", 
            PL_colors[4], PL_colors[5])
	);

  no_silent:
    if (no_final) {
        if (yes_state) {
            goto yes;
        } else {
            goto final_exit;
        }
    }    
    if (depth) {
	/* there's a previous state to backtrack to */
	st--;
	if (st < SLAB_FIRST(PL_regmatch_slab)) {
	    PL_regmatch_slab = PL_regmatch_slab->prev;
	    st = SLAB_LAST(PL_regmatch_slab);
	}
	PL_regmatch_state = st;
	locinput= st->locinput;

	DEBUG_STATE_pp("pop");
	depth--;
	if (yes_state == st)
	    yes_state = st->u.yes.prev_yes_state;

	state_num = st->resume_state + 1; /* failure = success + 1 */
	goto reenter_switch;
    }
    result = 0;

  final_exit:
    if (rex->intflags & PREGf_VERBARG_SEEN) {
        SV *sv_err = get_sv("REGERROR", 1);
        SV *sv_mrk = get_sv("REGMARK", 1);
        if (result) {
            sv_commit = &PL_sv_no;
            if (!sv_yes_mark) 
                sv_yes_mark = &PL_sv_yes;
        } else {
            if (!sv_commit) 
                sv_commit = &PL_sv_yes;
            sv_yes_mark = &PL_sv_no;
        }
        assert(sv_err);
        assert(sv_mrk);
        sv_setsv(sv_err, sv_commit);
        sv_setsv(sv_mrk, sv_yes_mark);
    }


    if (last_pushed_cv) {
	dSP;
	POP_MULTICALL;
        PERL_UNUSED_VAR(SP);
    }

    assert(!result ||  locinput - reginfo->strbeg >= 0);
    return result ?  locinput - reginfo->strbeg : -1;
}

/*
 - regrepeat - repeatedly match something simple, report how many
 *
 * What 'simple' means is a node which can be the operand of a quantifier like
 * '+', or {1,3}
 *
 * startposp - pointer a pointer to the start position.  This is updated
 *             to point to the byte following the highest successful
 *             match.
 * p         - the regnode to be repeatedly matched against.
 * reginfo   - struct holding match state, such as strend
 * max       - maximum number of things to match.
 * depth     - (for debugging) backtracking depth.
 */
STATIC I32
S_regrepeat(pTHX_ regexp *prog, char **startposp, const regnode *p,
            regmatch_info *const reginfo, I32 max, int depth)
{
    char *scan;     /* Pointer to current position in target string */
    I32 c;
    char *loceol = reginfo->strend;   /* local version */
    I32 hardcount = 0;  /* How many matches so far */
    bool utf8_target = reginfo->is_utf8_target;
    unsigned int to_complement = 0;  /* Invert the result? */
    UV utf8_flags;
    _char_class_number classnum;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    PERL_ARGS_ASSERT_REGREPEAT;

    scan = *startposp;
    if (max == REG_INFTY)
	max = I32_MAX;
    else if (! utf8_target && loceol - scan > max)
	loceol = scan + max;

    /* Here, for the case of a non-UTF-8 target we have adjusted <loceol> down
     * to the maximum of how far we should go in it (leaving it set to the real
     * end, if the maximum permissible would take us beyond that).  This allows
     * us to make the loop exit condition that we haven't gone past <loceol> to
     * also mean that we haven't exceeded the max permissible count, saving a
     * test each time through the loop.  But it assumes that the OP matches a
     * single byte, which is true for most of the OPs below when applied to a
     * non-UTF-8 target.  Those relatively few OPs that don't have this
     * characteristic will have to compensate.
     *
     * There is no adjustment for UTF-8 targets, as the number of bytes per
     * character varies.  OPs will have to test both that the count is less
     * than the max permissible (using <hardcount> to keep track), and that we
     * are still within the bounds of the string (using <loceol>.  A few OPs
     * match a single byte no matter what the encoding.  They can omit the max
     * test if, for the UTF-8 case, they do the adjustment that was skipped
     * above.
     *
     * Thus, the code above sets things up for the common case; and exceptional
     * cases need extra work; the common case is to make sure <scan> doesn't
     * go past <loceol>, and for UTF-8 to also use <hardcount> to make sure the
     * count doesn't exceed the maximum permissible */

    switch (OP(p)) {
    case REG_ANY:
	if (utf8_target) {
	    while (scan < loceol && hardcount < max && *scan != '\n') {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && *scan != '\n')
		scan++;
	}
	break;
    case SANY:
        if (utf8_target) {
	    while (scan < loceol && hardcount < max) {
	        scan += UTF8SKIP(scan);
		hardcount++;
	    }
	}
	else
	    scan = loceol;
	break;
    case CANY:  /* Move <scan> forward <max> bytes, unless goes off end */
        if (utf8_target && loceol - scan > max) {

            /* <loceol> hadn't been adjusted in the UTF-8 case */
            scan +=  max;
        }
        else {
            scan = loceol;
        }
	break;
    case EXACTL:
        _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
        if (utf8_target && UTF8_IS_ABOVE_LATIN1(*scan)) {
            _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(scan, loceol);
        }
        /* FALLTHROUGH */
    case EXACT:
        assert(STR_LEN(p) == reginfo->is_utf8_pat ? UTF8SKIP(STRING(p)) : 1);

	c = (U8)*STRING(p);

        /* Can use a simple loop if the pattern char to match on is invariant
         * under UTF-8, or both target and pattern aren't UTF-8.  Note that we
         * can use UTF8_IS_INVARIANT() even if the pattern isn't UTF-8, as it's
         * true iff it doesn't matter if the argument is in UTF-8 or not */
        if (UTF8_IS_INVARIANT(c) || (! utf8_target && ! reginfo->is_utf8_pat)) {
            if (utf8_target && loceol - scan > max) {
                /* We didn't adjust <loceol> because is UTF-8, but ok to do so,
                 * since here, to match at all, 1 char == 1 byte */
                loceol = scan + max;
            }
	    while (scan < loceol && UCHARAT(scan) == c) {
		scan++;
	    }
	}
	else if (reginfo->is_utf8_pat) {
            if (utf8_target) {
                STRLEN scan_char_len;

                /* When both target and pattern are UTF-8, we have to do
                 * string EQ */
                while (hardcount < max
                       && scan < loceol
                       && (scan_char_len = UTF8SKIP(scan)) <= STR_LEN(p)
                       && memEQ(scan, STRING(p), scan_char_len))
                {
                    scan += scan_char_len;
                    hardcount++;
                }
            }
            else if (! UTF8_IS_ABOVE_LATIN1(c)) {

                /* Target isn't utf8; convert the character in the UTF-8
                 * pattern to non-UTF8, and do a simple loop */
                c = TWO_BYTE_UTF8_TO_NATIVE(c, *(STRING(p) + 1));
                while (scan < loceol && UCHARAT(scan) == c) {
                    scan++;
                }
            } /* else pattern char is above Latin1, can't possibly match the
                 non-UTF-8 target */
        }
        else {

            /* Here, the string must be utf8; pattern isn't, and <c> is
             * different in utf8 than not, so can't compare them directly.
             * Outside the loop, find the two utf8 bytes that represent c, and
             * then look for those in sequence in the utf8 string */
	    U8 high = UTF8_TWO_BYTE_HI(c);
	    U8 low = UTF8_TWO_BYTE_LO(c);

	    while (hardcount < max
		    && scan + 1 < loceol
		    && UCHARAT(scan) == high
		    && UCHARAT(scan + 1) == low)
	    {
		scan += 2;
		hardcount++;
	    }
	}
	break;

    case EXACTFA_NO_TRIE:   /* This node only generated for non-utf8 patterns */
        assert(! reginfo->is_utf8_pat);
        /* FALLTHROUGH */
    case EXACTFA:
        utf8_flags = FOLDEQ_UTF8_NOMIX_ASCII;
	goto do_exactf;

    case EXACTFL:
        _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
	utf8_flags = FOLDEQ_LOCALE;
	goto do_exactf;

    case EXACTF:   /* This node only generated for non-utf8 patterns */
        assert(! reginfo->is_utf8_pat);
        utf8_flags = 0;
        goto do_exactf;

    case EXACTFLU8:
        if (! utf8_target) {
            break;
        }
        utf8_flags =  FOLDEQ_LOCALE | FOLDEQ_S2_ALREADY_FOLDED
                                    | FOLDEQ_S2_FOLDS_SANE;
        goto do_exactf;

    case EXACTFU_SS:
    case EXACTFU:
	utf8_flags = reginfo->is_utf8_pat ? FOLDEQ_S2_ALREADY_FOLDED : 0;

      do_exactf: {
        int c1, c2;
        U8 c1_utf8[UTF8_MAXBYTES+1], c2_utf8[UTF8_MAXBYTES+1];

        assert(STR_LEN(p) == reginfo->is_utf8_pat ? UTF8SKIP(STRING(p)) : 1);

        if (S_setup_EXACTISH_ST_c1_c2(aTHX_ p, &c1, c1_utf8, &c2, c2_utf8,
                                        reginfo))
        {
            if (c1 == CHRTEST_VOID) {
                /* Use full Unicode fold matching */
                char *tmpeol = reginfo->strend;
                STRLEN pat_len = reginfo->is_utf8_pat ? UTF8SKIP(STRING(p)) : 1;
                while (hardcount < max
                        && foldEQ_utf8_flags(scan, &tmpeol, 0, utf8_target,
                                             STRING(p), NULL, pat_len,
                                             reginfo->is_utf8_pat, utf8_flags))
                {
                    scan = tmpeol;
                    tmpeol = reginfo->strend;
                    hardcount++;
                }
            }
            else if (utf8_target) {
                if (c1 == c2) {
                    while (scan < loceol
                           && hardcount < max
                           && memEQ(scan, c1_utf8, UTF8SKIP(scan)))
                    {
                        scan += UTF8SKIP(scan);
                        hardcount++;
                    }
                }
                else {
                    while (scan < loceol
                           && hardcount < max
                           && (memEQ(scan, c1_utf8, UTF8SKIP(scan))
                               || memEQ(scan, c2_utf8, UTF8SKIP(scan))))
                    {
                        scan += UTF8SKIP(scan);
                        hardcount++;
                    }
                }
            }
            else if (c1 == c2) {
                while (scan < loceol && UCHARAT(scan) == c1) {
                    scan++;
                }
            }
            else {
                while (scan < loceol &&
                    (UCHARAT(scan) == c1 || UCHARAT(scan) == c2))
                {
                    scan++;
                }
            }
	}
	break;
    }
    case ANYOFL:
        _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
        /* FALLTHROUGH */
    case ANYOF:
	if (utf8_target) {
	    while (hardcount < max
                   && scan < loceol
		   && reginclass(prog, p, (U8*)scan, (U8*) loceol, utf8_target))
	    {
		scan += UTF8SKIP(scan);
		hardcount++;
	    }
	} else {
	    while (scan < loceol && REGINCLASS(prog, p, (U8*)scan))
		scan++;
	}
	break;

    /* The argument (FLAGS) to all the POSIX node types is the class number */

    case NPOSIXL:
        to_complement = 1;
        /* FALLTHROUGH */

    case POSIXL:
        _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
	if (! utf8_target) {
	    while (scan < loceol && to_complement ^ cBOOL(isFOO_lc(FLAGS(p),
                                                                   *scan)))
            {
		scan++;
            }
	} else {
	    while (hardcount < max && scan < loceol
                   && to_complement ^ cBOOL(isFOO_utf8_lc(FLAGS(p),
                                                                  (U8 *) scan)))
            {
                scan += UTF8SKIP(scan);
		hardcount++;
	    }
	}
	break;

    case POSIXD:
        if (utf8_target) {
            goto utf8_posix;
        }
        /* FALLTHROUGH */

    case POSIXA:
        if (utf8_target && loceol - scan > max) {

            /* We didn't adjust <loceol> at the beginning of this routine
             * because is UTF-8, but it is actually ok to do so, since here, to
             * match, 1 char == 1 byte. */
            loceol = scan + max;
        }
        while (scan < loceol && _generic_isCC_A((U8) *scan, FLAGS(p))) {
	    scan++;
	}
	break;

    case NPOSIXD:
        if (utf8_target) {
            to_complement = 1;
            goto utf8_posix;
        }
        /* FALLTHROUGH */

    case NPOSIXA:
        if (! utf8_target) {
            while (scan < loceol && ! _generic_isCC_A((U8) *scan, FLAGS(p))) {
                scan++;
            }
        }
        else {

            /* The complement of something that matches only ASCII matches all
             * non-ASCII, plus everything in ASCII that isn't in the class. */
	    while (hardcount < max && scan < loceol
                   && (! isASCII_utf8(scan)
                       || ! _generic_isCC_A((U8) *scan, FLAGS(p))))
            {
                scan += UTF8SKIP(scan);
		hardcount++;
	    }
        }
        break;

    case NPOSIXU:
        to_complement = 1;
        /* FALLTHROUGH */

    case POSIXU:
	if (! utf8_target) {
            while (scan < loceol && to_complement
                                ^ cBOOL(_generic_isCC((U8) *scan, FLAGS(p))))
            {
                scan++;
            }
	}
	else {
          utf8_posix:
            classnum = (_char_class_number) FLAGS(p);
            if (classnum < _FIRST_NON_SWASH_CC) {

                /* Here, a swash is needed for above-Latin1 code points.
                 * Process as many Latin1 code points using the built-in rules.
                 * Go to another loop to finish processing upon encountering
                 * the first Latin1 code point.  We could do that in this loop
                 * as well, but the other way saves having to test if the swash
                 * has been loaded every time through the loop: extra space to
                 * save a test. */
                while (hardcount < max && scan < loceol) {
                    if (UTF8_IS_INVARIANT(*scan)) {
                        if (! (to_complement ^ cBOOL(_generic_isCC((U8) *scan,
                                                                   classnum))))
                        {
                            break;
                        }
                        scan++;
                    }
                    else if (UTF8_IS_DOWNGRADEABLE_START(*scan)) {
                        if (! (to_complement
                              ^ cBOOL(_generic_isCC(TWO_BYTE_UTF8_TO_NATIVE(*scan,
                                                                     *(scan + 1)),
                                                    classnum))))
                        {
                            break;
                        }
                        scan += 2;
                    }
                    else {
                        goto found_above_latin1;
                    }

                    hardcount++;
                }
            }
            else {
                /* For these character classes, the knowledge of how to handle
                 * every code point is compiled in to Perl via a macro.  This
                 * code is written for making the loops as tight as possible.
                 * It could be refactored to save space instead */
                switch (classnum) {
                    case _CC_ENUM_SPACE:
                        while (hardcount < max
                               && scan < loceol
                               && (to_complement ^ cBOOL(isSPACE_utf8(scan))))
                        {
                            scan += UTF8SKIP(scan);
                            hardcount++;
                        }
                        break;
                    case _CC_ENUM_BLANK:
                        while (hardcount < max
                               && scan < loceol
                               && (to_complement ^ cBOOL(isBLANK_utf8(scan))))
                        {
                            scan += UTF8SKIP(scan);
                            hardcount++;
                        }
                        break;
                    case _CC_ENUM_XDIGIT:
                        while (hardcount < max
                               && scan < loceol
                               && (to_complement ^ cBOOL(isXDIGIT_utf8(scan))))
                        {
                            scan += UTF8SKIP(scan);
                            hardcount++;
                        }
                        break;
                    case _CC_ENUM_VERTSPACE:
                        while (hardcount < max
                               && scan < loceol
                               && (to_complement ^ cBOOL(isVERTWS_utf8(scan))))
                        {
                            scan += UTF8SKIP(scan);
                            hardcount++;
                        }
                        break;
                    case _CC_ENUM_CNTRL:
                        while (hardcount < max
                               && scan < loceol
                               && (to_complement ^ cBOOL(isCNTRL_utf8(scan))))
                        {
                            scan += UTF8SKIP(scan);
                            hardcount++;
                        }
                        break;
                    default:
                        Perl_croak(aTHX_ "panic: regrepeat() node %d='%s' has an unexpected character class '%d'", OP(p), PL_reg_name[OP(p)], classnum);
                }
            }
	}
        break;

      found_above_latin1:   /* Continuation of POSIXU and NPOSIXU */

        /* Load the swash if not already present */
        if (! PL_utf8_swash_ptrs[classnum]) {
            U8 flags = _CORE_SWASH_INIT_ACCEPT_INVLIST;
            PL_utf8_swash_ptrs[classnum] = _core_swash_init(
                                        "utf8",
                                        "",
                                        &PL_sv_undef, 1, 0,
                                        PL_XPosix_ptrs[classnum], &flags);
        }

        while (hardcount < max && scan < loceol
               && to_complement ^ cBOOL(_generic_utf8(
                                       classnum,
                                       scan,
                                       swash_fetch(PL_utf8_swash_ptrs[classnum],
                                                   (U8 *) scan,
                                                   TRUE))))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

    case LNBREAK:
        if (utf8_target) {
	    while (hardcount < max && scan < loceol &&
                    (c=is_LNBREAK_utf8_safe(scan, loceol))) {
		scan += c;
		hardcount++;
	    }
	} else {
            /* LNBREAK can match one or two latin chars, which is ok, but we
             * have to use hardcount in this situation, and throw away the
             * adjustment to <loceol> done before the switch statement */
            loceol = reginfo->strend;
	    while (scan < loceol && (c=is_LNBREAK_latin1_safe(scan, loceol))) {
		scan+=c;
		hardcount++;
	    }
	}
	break;

    case BOUNDL:
    case NBOUNDL:
        _CHECK_AND_WARN_PROBLEMATIC_LOCALE;
        /* FALLTHROUGH */
    case BOUND:
    case BOUNDA:
    case BOUNDU:
    case EOS:
    case GPOS:
    case KEEPS:
    case NBOUND:
    case NBOUNDA:
    case NBOUNDU:
    case OPFAIL:
    case SBOL:
    case SEOL:
        /* These are all 0 width, so match right here or not at all. */
        break;

    default:
        Perl_croak(aTHX_ "panic: regrepeat() called with unrecognized node type %d='%s'", OP(p), PL_reg_name[OP(p)]);
        /* NOTREACHED */
        NOT_REACHED; /* NOTREACHED */

    }

    if (hardcount)
	c = hardcount;
    else
	c = scan - *startposp;
    *startposp = scan;

    DEBUG_r({
	GET_RE_DEBUG_FLAGS_DECL;
	DEBUG_EXECUTE_r({
	    SV * const prop = sv_newmortal();
            regprop(prog, prop, p, reginfo, NULL);
	    PerlIO_printf(Perl_debug_log,
			"%*s  %s can match %"IVdf" times out of %"IVdf"...\n",
			REPORT_CODE_OFF + depth*2, "", SvPVX_const(prop),(IV)c,(IV)max);
	});
    });

    return(c);
}


#if !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION)
/*
- regclass_swash - prepare the utf8 swash.  Wraps the shared core version to
create a copy so that changes the caller makes won't change the shared one.
If <altsvp> is non-null, will return NULL in it, for back-compat.
 */
SV *
Perl_regclass_swash(pTHX_ const regexp *prog, const regnode* node, bool doinit, SV** listsvp, SV **altsvp)
{
    PERL_ARGS_ASSERT_REGCLASS_SWASH;

    if (altsvp) {
        *altsvp = NULL;
    }

    return newSVsv(_get_regclass_nonbitmap_data(prog, node, doinit, listsvp, NULL, NULL));
}

#endif /* !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION) */

/*
 - reginclass - determine if a character falls into a character class
 
  n is the ANYOF-type regnode
  p is the target string
  p_end points to one byte beyond the end of the target string
  utf8_target tells whether p is in UTF-8.

  Returns true if matched; false otherwise.

  Note that this can be a synthetic start class, a combination of various
  nodes, so things you think might be mutually exclusive, such as locale,
  aren't.  It can match both locale and non-locale

 */

STATIC bool
S_reginclass(pTHX_ regexp * const prog, const regnode * const n, const U8* const p, const U8* const p_end, const bool utf8_target)
{
    dVAR;
    const char flags = ANYOF_FLAGS(n);
    bool match = FALSE;
    UV c = *p;

    PERL_ARGS_ASSERT_REGINCLASS;

    /* If c is not already the code point, get it.  Note that
     * UTF8_IS_INVARIANT() works even if not in UTF-8 */
    if (! UTF8_IS_INVARIANT(c) && utf8_target) {
        STRLEN c_len = 0;
	c = utf8n_to_uvchr(p, p_end - p, &c_len,
		(UTF8_ALLOW_DEFAULT & UTF8_ALLOW_ANYUV)
		| UTF8_ALLOW_FFFF | UTF8_CHECK_ONLY);
		/* see [perl #37836] for UTF8_ALLOW_ANYUV; [perl #38293] for
		 * UTF8_ALLOW_FFFF */
	if (c_len == (STRLEN)-1)
	    Perl_croak(aTHX_ "Malformed UTF-8 character (fatal)");
        if (c > 255 && OP(n) == ANYOFL && ! is_ANYOF_SYNTHETIC(n)) {
            _CHECK_AND_OUTPUT_WIDE_LOCALE_CP_MSG(c);
        }
    }

    /* If this character is potentially in the bitmap, check it */
    if (c < NUM_ANYOF_CODE_POINTS) {
	if (ANYOF_BITMAP_TEST(n, c))
	    match = TRUE;
	else if ((flags & ANYOF_MATCHES_ALL_NON_UTF8_NON_ASCII)
		  && ! utf8_target
		  && ! isASCII(c))
	{
	    match = TRUE;
	}
	else if (flags & ANYOF_LOCALE_FLAGS) {
	    if ((flags & ANYOF_LOC_FOLD)
                && c < 256
		&& ANYOF_BITMAP_TEST(n, PL_fold_locale[c]))
            {
                match = TRUE;
            }
            else if (ANYOF_POSIXL_TEST_ANY_SET(n)
                     && c < 256
            ) {

                /* The data structure is arranged so bits 0, 2, 4, ... are set
                 * if the class includes the Posix character class given by
                 * bit/2; and 1, 3, 5, ... are set if the class includes the
                 * complemented Posix class given by int(bit/2).  So we loop
                 * through the bits, each time changing whether we complement
                 * the result or not.  Suppose for the sake of illustration
                 * that bits 0-3 mean respectively, \w, \W, \s, \S.  If bit 0
                 * is set, it means there is a match for this ANYOF node if the
                 * character is in the class given by the expression (0 / 2 = 0
                 * = \w).  If it is in that class, isFOO_lc() will return 1,
                 * and since 'to_complement' is 0, the result will stay TRUE,
                 * and we exit the loop.  Suppose instead that bit 0 is 0, but
                 * bit 1 is 1.  That means there is a match if the character
                 * matches \W.  We won't bother to call isFOO_lc() on bit 0,
                 * but will on bit 1.  On the second iteration 'to_complement'
                 * will be 1, so the exclusive or will reverse things, so we
                 * are testing for \W.  On the third iteration, 'to_complement'
                 * will be 0, and we would be testing for \s; the fourth
                 * iteration would test for \S, etc.
                 *
                 * Note that this code assumes that all the classes are closed
                 * under folding.  For example, if a character matches \w, then
                 * its fold does too; and vice versa.  This should be true for
                 * any well-behaved locale for all the currently defined Posix
                 * classes, except for :lower: and :upper:, which are handled
                 * by the pseudo-class :cased: which matches if either of the
                 * other two does.  To get rid of this assumption, an outer
                 * loop could be used below to iterate over both the source
                 * character, and its fold (if different) */

                int count = 0;
                int to_complement = 0;

                while (count < ANYOF_MAX) {
                    if (ANYOF_POSIXL_TEST(n, count)
                        && to_complement ^ cBOOL(isFOO_lc(count/2, (U8) c)))
                    {
                        match = TRUE;
                        break;
                    }
                    count++;
                    to_complement ^= 1;
                }
	    }
	}
    }


    /* If the bitmap didn't (or couldn't) match, and something outside the
     * bitmap could match, try that. */
    if (!match) {
	if (c >= NUM_ANYOF_CODE_POINTS
            && (flags & ANYOF_MATCHES_ALL_ABOVE_BITMAP))
        {
	    match = TRUE;	/* Everything above the bitmap matches */
	}
	else if ((flags & ANYOF_HAS_NONBITMAP_NON_UTF8_MATCHES)
		  || (utf8_target && (flags & ANYOF_HAS_UTF8_NONBITMAP_MATCHES))
                  || ((flags & ANYOF_LOC_FOLD)
                       && IN_UTF8_CTYPE_LOCALE
                       && ARG(n) != ANYOF_ONLY_HAS_BITMAP))
        {
            SV* only_utf8_locale = NULL;
	    SV * const sw = _get_regclass_nonbitmap_data(prog, n, TRUE, 0,
                                                       &only_utf8_locale, NULL);
	    if (sw) {
                U8 utf8_buffer[2];
		U8 * utf8_p;
		if (utf8_target) {
		    utf8_p = (U8 *) p;
		} else { /* Convert to utf8 */
		    utf8_p = utf8_buffer;
                    append_utf8_from_native_byte(*p, &utf8_p);
		    utf8_p = utf8_buffer;
		}

		if (swash_fetch(sw, utf8_p, TRUE)) {
		    match = TRUE;
                }
	    }
            if (! match && only_utf8_locale && IN_UTF8_CTYPE_LOCALE) {
                match = _invlist_contains_cp(only_utf8_locale, c);
            }
	}

        if (UNICODE_IS_SUPER(c)
            && (flags & ANYOF_WARN_SUPER)
            && ckWARN_d(WARN_NON_UNICODE))
        {
            Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
                "Matched non-Unicode code point 0x%04"UVXf" against Unicode property; may not be portable", c);
        }
    }

#if ANYOF_INVERT != 1
    /* Depending on compiler optimization cBOOL takes time, so if don't have to
     * use it, don't */
#   error ANYOF_INVERT needs to be set to 1, or guarded with cBOOL below,
#endif

    /* The xor complements the return if to invert: 1^1 = 0, 1^0 = 1 */
    return (flags & ANYOF_INVERT) ^ match;
}

STATIC U8 *
S_reghop3(U8 *s, SSize_t off, const U8* lim)
{
    /* return the position 'off' UTF-8 characters away from 's', forward if
     * 'off' >= 0, backwards if negative.  But don't go outside of position
     * 'lim', which better be < s  if off < 0 */

    PERL_ARGS_ASSERT_REGHOP3;

    if (off >= 0) {
	while (off-- && s < lim) {
	    /* XXX could check well-formedness here */
	    s += UTF8SKIP(s);
	}
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
	    }
            /* XXX could check well-formedness here */
	}
    }
    return s;
}

STATIC U8 *
S_reghop4(U8 *s, SSize_t off, const U8* llim, const U8* rlim)
{
    PERL_ARGS_ASSERT_REGHOP4;

    if (off >= 0) {
        while (off-- && s < rlim) {
            /* XXX could check well-formedness here */
            s += UTF8SKIP(s);
        }
    }
    else {
        while (off++ && s > llim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > llim && UTF8_IS_CONTINUATION(*s))
                    s--;
            }
            /* XXX could check well-formedness here */
        }
    }
    return s;
}

/* like reghop3, but returns NULL on overrun, rather than returning last
 * char pos */

STATIC U8 *
S_reghopmaybe3(U8* s, SSize_t off, const U8* lim)
{
    PERL_ARGS_ASSERT_REGHOPMAYBE3;

    if (off >= 0) {
	while (off-- && s < lim) {
	    /* XXX could check well-formedness here */
	    s += UTF8SKIP(s);
	}
	if (off >= 0)
	    return NULL;
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
	    }
            /* XXX could check well-formedness here */
	}
	if (off <= 0)
	    return NULL;
    }
    return s;
}


/* when executing a regex that may have (?{}), extra stuff needs setting
   up that will be visible to the called code, even before the current
   match has finished. In particular:

   * $_ is localised to the SV currently being matched;
   * pos($_) is created if necessary, ready to be updated on each call-out
     to code;
   * a fake PMOP is created that can be set to PL_curpm (normally PL_curpm
     isn't set until the current pattern is successfully finished), so that
     $1 etc of the match-so-far can be seen;
   * save the old values of subbeg etc of the current regex, and  set then
     to the current string (again, this is normally only done at the end
     of execution)
*/

static void
S_setup_eval_state(pTHX_ regmatch_info *const reginfo)
{
    MAGIC *mg;
    regexp *const rex = ReANY(reginfo->prog);
    regmatch_info_aux_eval *eval_state = reginfo->info_aux_eval;

    eval_state->rex = rex;

    if (reginfo->sv) {
        /* Make $_ available to executed code. */
        if (reginfo->sv != DEFSV) {
            SAVE_DEFSV;
            DEFSV_set(reginfo->sv);
        }

        if (!(mg = mg_find_mglob(reginfo->sv))) {
            /* prepare for quick setting of pos */
            mg = sv_magicext_mglob(reginfo->sv);
            mg->mg_len = -1;
        }
        eval_state->pos_magic = mg;
        eval_state->pos       = mg->mg_len;
        eval_state->pos_flags = mg->mg_flags;
    }
    else
        eval_state->pos_magic = NULL;

    if (!PL_reg_curpm) {
        /* PL_reg_curpm is a fake PMOP that we can attach the current
         * regex to and point PL_curpm at, so that $1 et al are visible
         * within a /(?{})/. It's just allocated once per interpreter the
         * first time its needed */
        Newxz(PL_reg_curpm, 1, PMOP);
#ifdef USE_ITHREADS
        {
            SV* const repointer = &PL_sv_undef;
            /* this regexp is also owned by the new PL_reg_curpm, which
               will try to free it.  */
            av_push(PL_regex_padav, repointer);
            PL_reg_curpm->op_pmoffset = av_tindex(PL_regex_padav);
            PL_regex_pad = AvARRAY(PL_regex_padav);
        }
#endif
    }
    SET_reg_curpm(reginfo->prog);
    eval_state->curpm = PL_curpm;
    PL_curpm = PL_reg_curpm;
    if (RXp_MATCH_COPIED(rex)) {
        /*  Here is a serious problem: we cannot rewrite subbeg,
            since it may be needed if this match fails.  Thus
            $` inside (?{}) could fail... */
        eval_state->subbeg     = rex->subbeg;
        eval_state->sublen     = rex->sublen;
        eval_state->suboffset  = rex->suboffset;
        eval_state->subcoffset = rex->subcoffset;
#ifdef PERL_ANY_COW
        eval_state->saved_copy = rex->saved_copy;
#endif
        RXp_MATCH_COPIED_off(rex);
    }
    else
        eval_state->subbeg = NULL;
    rex->subbeg = (char *)reginfo->strbeg;
    rex->suboffset = 0;
    rex->subcoffset = 0;
    rex->sublen = reginfo->strend - reginfo->strbeg;
}


/* destructor to clear up regmatch_info_aux and regmatch_info_aux_eval */

static void
S_cleanup_regmatch_info_aux(pTHX_ void *arg)
{
    regmatch_info_aux *aux = (regmatch_info_aux *) arg;
    regmatch_info_aux_eval *eval_state =  aux->info_aux_eval;
    regmatch_slab *s;

    Safefree(aux->poscache);

    if (eval_state) {

        /* undo the effects of S_setup_eval_state() */

        if (eval_state->subbeg) {
            regexp * const rex = eval_state->rex;
            rex->subbeg     = eval_state->subbeg;
            rex->sublen     = eval_state->sublen;
            rex->suboffset  = eval_state->suboffset;
            rex->subcoffset = eval_state->subcoffset;
#ifdef PERL_ANY_COW
            rex->saved_copy = eval_state->saved_copy;
#endif
            RXp_MATCH_COPIED_on(rex);
        }
        if (eval_state->pos_magic)
        {
            eval_state->pos_magic->mg_len = eval_state->pos;
            eval_state->pos_magic->mg_flags =
                 (eval_state->pos_magic->mg_flags & ~MGf_BYTES)
               | (eval_state->pos_flags & MGf_BYTES);
        }

        PL_curpm = eval_state->curpm;
    }

    PL_regmatch_state = aux->old_regmatch_state;
    PL_regmatch_slab  = aux->old_regmatch_slab;

    /* free all slabs above current one - this must be the last action
     * of this function, as aux and eval_state are allocated within
     * slabs and may be freed here */

    s = PL_regmatch_slab->next;
    if (s) {
        PL_regmatch_slab->next = NULL;
        while (s) {
            regmatch_slab * const osl = s;
            s = s->next;
            Safefree(osl);
        }
    }
}


STATIC void
S_to_utf8_substr(pTHX_ regexp *prog)
{
    /* Converts substr fields in prog from bytes to UTF-8, calling fbm_compile
     * on the converted value */

    int i = 1;

    PERL_ARGS_ASSERT_TO_UTF8_SUBSTR;

    do {
	if (prog->substrs->data[i].substr
	    && !prog->substrs->data[i].utf8_substr) {
	    SV* const sv = newSVsv(prog->substrs->data[i].substr);
	    prog->substrs->data[i].utf8_substr = sv;
	    sv_utf8_upgrade(sv);
	    if (SvVALID(prog->substrs->data[i].substr)) {
		if (SvTAIL(prog->substrs->data[i].substr)) {
		    /* Trim the trailing \n that fbm_compile added last
		       time.  */
		    SvCUR_set(sv, SvCUR(sv) - 1);
		    /* Whilst this makes the SV technically "invalid" (as its
		       buffer is no longer followed by "\0") when fbm_compile()
		       adds the "\n" back, a "\0" is restored.  */
		    fbm_compile(sv, FBMcf_TAIL);
		} else
		    fbm_compile(sv, 0);
	    }
	    if (prog->substrs->data[i].substr == prog->check_substr)
		prog->check_utf8 = sv;
	}
    } while (i--);
}

STATIC bool
S_to_byte_substr(pTHX_ regexp *prog)
{
    /* Converts substr fields in prog from UTF-8 to bytes, calling fbm_compile
     * on the converted value; returns FALSE if can't be converted. */

    int i = 1;

    PERL_ARGS_ASSERT_TO_BYTE_SUBSTR;

    do {
	if (prog->substrs->data[i].utf8_substr
	    && !prog->substrs->data[i].substr) {
	    SV* sv = newSVsv(prog->substrs->data[i].utf8_substr);
	    if (! sv_utf8_downgrade(sv, TRUE)) {
                return FALSE;
            }
            if (SvVALID(prog->substrs->data[i].utf8_substr)) {
                if (SvTAIL(prog->substrs->data[i].utf8_substr)) {
                    /* Trim the trailing \n that fbm_compile added last
                        time.  */
                    SvCUR_set(sv, SvCUR(sv) - 1);
                    fbm_compile(sv, FBMcf_TAIL);
                } else
                    fbm_compile(sv, 0);
            }
	    prog->substrs->data[i].substr = sv;
	    if (prog->substrs->data[i].utf8_substr == prog->check_utf8)
		prog->check_substr = sv;
	}
    } while (i--);

    return TRUE;
}