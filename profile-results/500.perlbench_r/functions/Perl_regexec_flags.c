/*
 - regexec_flags - match a regexp against a string
 */
I32
Perl_regexec_flags(pTHX_ REGEXP * const rx, char *stringarg, char *strend,
	      char *strbeg, SSize_t minend, SV *sv, void *data, U32 flags)
/* stringarg: the point in the string at which to begin matching */
/* strend:    pointer to null at end of string */
/* strbeg:    real beginning of string */
/* minend:    end of match must be >= minend bytes after stringarg. */
/* sv:        SV being matched: only used for utf8 flag, pos() etc; string
 *            itself is accessed via the pointers above */
/* data:      May be used for some additional optimizations.
              Currently unused. */
/* flags:     For optimizations. See REXEC_* in regexp.h */

{
    struct regexp *const prog = ReANY(rx);
    char *s;
    regnode *c;
    char *startpos;
    SSize_t minlen;		/* must match at least this many chars */
    SSize_t dontbother = 0;	/* how many characters not to try at end */
    const bool utf8_target = cBOOL(DO_UTF8(sv));
    I32 multiline;
    RXi_GET_DECL(prog,progi);
    regmatch_info reginfo_buf;  /* create some info to pass to regtry etc */
    regmatch_info *const reginfo = &reginfo_buf;
    regexp_paren_pair *swap = NULL;
    I32 oldsave;
    GET_RE_DEBUG_FLAGS_DECL;

    PERL_ARGS_ASSERT_REGEXEC_FLAGS;
    PERL_UNUSED_ARG(data);

    /* Be paranoid... */
    if (prog == NULL) {
	Perl_croak(aTHX_ "NULL regexp parameter");
    }

    DEBUG_EXECUTE_r(
        debug_start_match(rx, utf8_target, stringarg, strend,
        "Matching");
    );

    startpos = stringarg;

    if (prog->intflags & PREGf_GPOS_SEEN) {
        MAGIC *mg;

        /* set reginfo->ganch, the position where \G can match */

        reginfo->ganch =
            (flags & REXEC_IGNOREPOS)
            ? stringarg /* use start pos rather than pos() */
            : ((mg = mg_find_mglob(sv)) && mg->mg_len >= 0)
              /* Defined pos(): */
            ? strbeg + MgBYTEPOS(mg, sv, strbeg, strend-strbeg)
            : strbeg; /* pos() not defined; use start of string */

        DEBUG_GPOS_r(PerlIO_printf(Perl_debug_log,
            "GPOS ganch set to strbeg[%"IVdf"]\n", (IV)(reginfo->ganch - strbeg)));

        /* in the presence of \G, we may need to start looking earlier in
         * the string than the suggested start point of stringarg:
         * if prog->gofs is set, then that's a known, fixed minimum
         * offset, such as
         * /..\G/:   gofs = 2
         * /ab|c\G/: gofs = 1
         * or if the minimum offset isn't known, then we have to go back
         * to the start of the string, e.g. /w+\G/
         */

        if (prog->intflags & PREGf_ANCH_GPOS) {
            startpos  = reginfo->ganch - prog->gofs;
            if (startpos <
                ((flags & REXEC_FAIL_ON_UNDERFLOW) ? stringarg : strbeg))
            {
                DEBUG_r(PerlIO_printf(Perl_debug_log,
                        "fail: ganch-gofs before earliest possible start\n"));
                return 0;
            }
        }
        else if (prog->gofs) {
            if (startpos - prog->gofs < strbeg)
                startpos = strbeg;
            else
                startpos -= prog->gofs;
        }
        else if (prog->intflags & PREGf_GPOS_FLOAT)
            startpos = strbeg;
    }

    minlen = prog->minlen;
    if ((startpos + minlen) > strend || startpos < strbeg) {
        DEBUG_r(PerlIO_printf(Perl_debug_log,
                    "Regex match can't succeed, so not even tried\n"));
        return 0;
    }

    /* at the end of this function, we'll do a LEAVE_SCOPE(oldsave),
     * which will call destuctors to reset PL_regmatch_state, free higher
     * PL_regmatch_slabs, and clean up regmatch_info_aux and
     * regmatch_info_aux_eval */

    oldsave = PL_savestack_ix;

    s = startpos;

    if ((prog->extflags & RXf_USE_INTUIT)
        && !(flags & REXEC_CHECKED))
    {
	s = re_intuit_start(rx, sv, strbeg, startpos, strend,
                                    flags, NULL);
	if (!s)
	    return 0;

	if (prog->extflags & RXf_CHECK_ALL) {
            /* we can match based purely on the result of INTUIT.
             * Set up captures etc just for $& and $-[0]
             * (an intuit-only match wont have $1,$2,..) */
            assert(!prog->nparens);

            /* s/// doesn't like it if $& is earlier than where we asked it to
             * start searching (which can happen on something like /.\G/) */
            if (       (flags & REXEC_FAIL_ON_UNDERFLOW)
                    && (s < stringarg))
            {
                /* this should only be possible under \G */
                assert(prog->intflags & PREGf_GPOS_SEEN);
                DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
                    "matched, but failing for REXEC_FAIL_ON_UNDERFLOW\n"));
                goto phooey;
            }

            /* match via INTUIT shouldn't have any captures.
             * Let @-, @+, $^N know */
            prog->lastparen = prog->lastcloseparen = 0;
            RX_MATCH_UTF8_set(rx, utf8_target);
            prog->offs[0].start = s - strbeg;
            prog->offs[0].end = utf8_target
                ? (char*)utf8_hop((U8*)s, prog->minlenret) - strbeg
                : s - strbeg + prog->minlenret;
            if ( !(flags & REXEC_NOT_FIRST) )
                S_reg_set_capture_string(aTHX_ rx,
                                        strbeg, strend,
                                        sv, flags, utf8_target);

	    return 1;
        }
    }

    multiline = prog->extflags & RXf_PMf_MULTILINE;
    
    if (strend - s < (minlen+(prog->check_offset_min<0?prog->check_offset_min:0))) {
        DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
			      "String too short [regexec_flags]...\n"));
	goto phooey;
    }
    
    /* Check validity of program. */
    if (UCHARAT(progi->program) != REG_MAGIC) {
	Perl_croak(aTHX_ "corrupted regexp program");
    }

    RX_MATCH_TAINTED_off(rx);
    RX_MATCH_UTF8_set(rx, utf8_target);

    reginfo->prog = rx;	 /* Yes, sorry that this is confusing.  */
    reginfo->intuit = 0;
    reginfo->is_utf8_target = cBOOL(utf8_target);
    reginfo->is_utf8_pat = cBOOL(RX_UTF8(rx));
    reginfo->warned = FALSE;
    reginfo->strbeg  = strbeg;
    reginfo->sv = sv;
    reginfo->poscache_maxiter = 0; /* not yet started a countdown */
    reginfo->strend = strend;
    /* see how far we have to get to not match where we matched before */
    reginfo->till = stringarg + minend;

    if (prog->extflags & RXf_EVAL_SEEN && SvPADTMP(sv)) {
        /* SAVEFREESV, not sv_mortalcopy, as this SV must last until after
           S_cleanup_regmatch_info_aux has executed (registered by
           SAVEDESTRUCTOR_X below).  S_cleanup_regmatch_info_aux modifies
           magic belonging to this SV.
           Not newSVsv, either, as it does not COW.
        */
        reginfo->sv = newSV(0);
        SvSetSV_nosteal(reginfo->sv, sv);
        SAVEFREESV(reginfo->sv);
    }

    /* reserve next 2 or 3 slots in PL_regmatch_state:
     * slot N+0: may currently be in use: skip it
     * slot N+1: use for regmatch_info_aux struct
     * slot N+2: use for regmatch_info_aux_eval struct if we have (?{})'s
     * slot N+3: ready for use by regmatch()
     */

    {
        regmatch_state *old_regmatch_state;
        regmatch_slab  *old_regmatch_slab;
        int i, max = (prog->extflags & RXf_EVAL_SEEN) ? 2 : 1;

        /* on first ever match, allocate first slab */
        if (!PL_regmatch_slab) {
            Newx(PL_regmatch_slab, 1, regmatch_slab);
            PL_regmatch_slab->prev = NULL;
            PL_regmatch_slab->next = NULL;
            PL_regmatch_state = SLAB_FIRST(PL_regmatch_slab);
        }

        old_regmatch_state = PL_regmatch_state;
        old_regmatch_slab  = PL_regmatch_slab;

        for (i=0; i <= max; i++) {
            if (i == 1)
                reginfo->info_aux = &(PL_regmatch_state->u.info_aux);
            else if (i ==2)
                reginfo->info_aux_eval =
                reginfo->info_aux->info_aux_eval =
                            &(PL_regmatch_state->u.info_aux_eval);

            if (++PL_regmatch_state >  SLAB_LAST(PL_regmatch_slab))
                PL_regmatch_state = S_push_slab(aTHX);
        }

        /* note initial PL_regmatch_state position; at end of match we'll
         * pop back to there and free any higher slabs */

        reginfo->info_aux->old_regmatch_state = old_regmatch_state;
        reginfo->info_aux->old_regmatch_slab  = old_regmatch_slab;
        reginfo->info_aux->poscache = NULL;

        SAVEDESTRUCTOR_X(S_cleanup_regmatch_info_aux, reginfo->info_aux);

        if ((prog->extflags & RXf_EVAL_SEEN))
            S_setup_eval_state(aTHX_ reginfo);
        else
            reginfo->info_aux_eval = reginfo->info_aux->info_aux_eval = NULL;
    }

    /* If there is a "must appear" string, look for it. */

    if (PL_curpm && (PM_GETRE(PL_curpm) == rx)) {
        /* We have to be careful. If the previous successful match
           was from this regex we don't want a subsequent partially
           successful match to clobber the old results.
           So when we detect this possibility we add a swap buffer
           to the re, and switch the buffer each match. If we fail,
           we switch it back; otherwise we leave it swapped.
        */
        swap = prog->offs;
        /* do we need a save destructor here for eval dies? */
        Newxz(prog->offs, (prog->nparens + 1), regexp_paren_pair);
	DEBUG_BUFFERS_r(PerlIO_printf(Perl_debug_log,
	    "rex=0x%"UVxf" saving  offs: orig=0x%"UVxf" new=0x%"UVxf"\n",
	    PTR2UV(prog),
	    PTR2UV(swap),
	    PTR2UV(prog->offs)
	));
    }

    /* Simplest case: anchored match need be tried only once, or with
     * MBOL, only at the beginning of each line.
     *
     * Note that /.*.../ sets PREGf_IMPLICIT|MBOL, while /.*.../s sets
     * PREGf_IMPLICIT|SBOL. The idea is that with /.*.../s, if it doesn't
     * match at the start of the string then it won't match anywhere else
     * either; while with /.*.../, if it doesn't match at the beginning,
     * the earliest it could match is at the start of the next line */

    if (prog->intflags & (PREGf_ANCH & ~PREGf_ANCH_GPOS)) {
        char *end;

	if (regtry(reginfo, &s))
	    goto got_it;

        if (!(prog->intflags & PREGf_ANCH_MBOL))
            goto phooey;

        /* didn't match at start, try at other newline positions */

        if (minlen)
            dontbother = minlen - 1;
        end = HOP3c(strend, -dontbother, strbeg) - 1;

        /* skip to next newline */

        while (s <= end) { /* note it could be possible to match at the end of the string */
            /* NB: newlines are the same in unicode as they are in latin */
            if (*s++ != '\n')
                continue;
            if (prog->check_substr || prog->check_utf8) {
            /* note that with PREGf_IMPLICIT, intuit can only fail
             * or return the start position, so it's of limited utility.
             * Nevertheless, I made the decision that the potential for
             * quick fail was still worth it - DAPM */
                s = re_intuit_start(rx, sv, strbeg, s, strend, flags, NULL);
                if (!s)
                    goto phooey;
            }
            if (regtry(reginfo, &s))
                goto got_it;
        }
        goto phooey;
    } /* end anchored search */

    if (prog->intflags & PREGf_ANCH_GPOS)
    {
        /* PREGf_ANCH_GPOS should never be true if PREGf_GPOS_SEEN is not true */
        assert(prog->intflags & PREGf_GPOS_SEEN);
        /* For anchored \G, the only position it can match from is
         * (ganch-gofs); we already set startpos to this above; if intuit
         * moved us on from there, we can't possibly succeed */
        assert(startpos == reginfo->ganch - prog->gofs);
	if (s == startpos && regtry(reginfo, &s))
	    goto got_it;
	goto phooey;
    }

    /* Messy cases:  unanchored match. */
    if ((prog->anchored_substr || prog->anchored_utf8) && prog->intflags & PREGf_SKIP) {
	/* we have /x+whatever/ */
	/* it must be a one character string (XXXX Except is_utf8_pat?) */
	char ch;
#ifdef DEBUGGING
	int did_match = 0;
#endif
	if (utf8_target) {
            if (! prog->anchored_utf8) {
                to_utf8_substr(prog);
            }
            ch = SvPVX_const(prog->anchored_utf8)[0];
	    REXEC_FBC_SCAN(
		if (*s == ch) {
		    DEBUG_EXECUTE_r( did_match = 1 );
		    if (regtry(reginfo, &s)) goto got_it;
		    s += UTF8SKIP(s);
		    while (s < strend && *s == ch)
			s += UTF8SKIP(s);
		}
	    );

	}
	else {
            if (! prog->anchored_substr) {
                if (! to_byte_substr(prog)) {
                    NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                }
            }
            ch = SvPVX_const(prog->anchored_substr)[0];
	    REXEC_FBC_SCAN(
		if (*s == ch) {
		    DEBUG_EXECUTE_r( did_match = 1 );
		    if (regtry(reginfo, &s)) goto got_it;
		    s++;
		    while (s < strend && *s == ch)
			s++;
		}
	    );
	}
	DEBUG_EXECUTE_r(if (!did_match)
		PerlIO_printf(Perl_debug_log,
                                  "Did not find anchored character...\n")
               );
    }
    else if (prog->anchored_substr != NULL
	      || prog->anchored_utf8 != NULL
	      || ((prog->float_substr != NULL || prog->float_utf8 != NULL)
		  && prog->float_max_offset < strend - s)) {
	SV *must;
	SSize_t back_max;
	SSize_t back_min;
	char *last;
	char *last1;		/* Last position checked before */
#ifdef DEBUGGING
	int did_match = 0;
#endif
	if (prog->anchored_substr || prog->anchored_utf8) {
	    if (utf8_target) {
                if (! prog->anchored_utf8) {
                    to_utf8_substr(prog);
                }
                must = prog->anchored_utf8;
            }
            else {
                if (! prog->anchored_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                must = prog->anchored_substr;
            }
	    back_max = back_min = prog->anchored_offset;
	} else {
	    if (utf8_target) {
                if (! prog->float_utf8) {
                    to_utf8_substr(prog);
                }
                must = prog->float_utf8;
            }
            else {
                if (! prog->float_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                must = prog->float_substr;
            }
	    back_max = prog->float_max_offset;
	    back_min = prog->float_min_offset;
	}
	    
        if (back_min<0) {
	    last = strend;
	} else {
            last = HOP3c(strend,	/* Cannot start after this */
        	  -(SSize_t)(CHR_SVLEN(must)
        		 - (SvTAIL(must) != 0) + back_min), strbeg);
        }
	if (s > reginfo->strbeg)
	    last1 = HOPc(s, -1);
	else
	    last1 = s - 1;	/* bogus */

	/* XXXX check_substr already used to find "s", can optimize if
	   check_substr==must. */
	dontbother = 0;
	strend = HOPc(strend, -dontbother);
	while ( (s <= last) &&
		(s = fbm_instr((unsigned char*)HOP4c(s, back_min, strbeg,  strend),
				  (unsigned char*)strend, must,
				  multiline ? FBMrf_MULTILINE : 0)) ) {
	    DEBUG_EXECUTE_r( did_match = 1 );
	    if (HOPc(s, -back_max) > last1) {
		last1 = HOPc(s, -back_min);
		s = HOPc(s, -back_max);
	    }
	    else {
		char * const t = (last1 >= reginfo->strbeg)
                                    ? HOPc(last1, 1) : last1 + 1;

		last1 = HOPc(s, -back_min);
		s = t;
	    }
	    if (utf8_target) {
		while (s <= last1) {
		    if (regtry(reginfo, &s))
			goto got_it;
                    if (s >= last1) {
                        s++; /* to break out of outer loop */
                        break;
                    }
                    s += UTF8SKIP(s);
		}
	    }
	    else {
		while (s <= last1) {
		    if (regtry(reginfo, &s))
			goto got_it;
		    s++;
		}
	    }
	}
	DEBUG_EXECUTE_r(if (!did_match) {
            RE_PV_QUOTED_DECL(quoted, utf8_target, PERL_DEBUG_PAD_ZERO(0),
                SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
            PerlIO_printf(Perl_debug_log, "Did not find %s substr %s%s...\n",
			      ((must == prog->anchored_substr || must == prog->anchored_utf8)
			       ? "anchored" : "floating"),
                quoted, RE_SV_TAIL(must));
        });		    
	goto phooey;
    }
    else if ( (c = progi->regstclass) ) {
	if (minlen) {
	    const OPCODE op = OP(progi->regstclass);
	    /* don't bother with what can't match */
	    if (PL_regkind[op] != EXACT && op != CANY && PL_regkind[op] != TRIE)
	        strend = HOPc(strend, -(minlen - 1));
	}
	DEBUG_EXECUTE_r({
	    SV * const prop = sv_newmortal();
            regprop(prog, prop, c, reginfo, NULL);
	    {
		RE_PV_QUOTED_DECL(quoted,utf8_target,PERL_DEBUG_PAD_ZERO(1),
		    s,strend-s,60);
		PerlIO_printf(Perl_debug_log,
		    "Matching stclass %.*s against %s (%d bytes)\n",
		    (int)SvCUR(prop), SvPVX_const(prop),
		     quoted, (int)(strend - s));
	    }
	});
        if (find_byclass(prog, c, s, strend, reginfo))
	    goto got_it;
	DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "Contradicts stclass... [regexec_flags]\n"));
    }
    else {
	dontbother = 0;
	if (prog->float_substr != NULL || prog->float_utf8 != NULL) {
	    /* Trim the end. */
	    char *last= NULL;
	    SV* float_real;
	    STRLEN len;
	    const char *little;

	    if (utf8_target) {
                if (! prog->float_utf8) {
                    to_utf8_substr(prog);
                }
                float_real = prog->float_utf8;
            }
            else {
                if (! prog->float_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                float_real = prog->float_substr;
            }

            little = SvPV_const(float_real, len);
	    if (SvTAIL(float_real)) {
                    /* This means that float_real contains an artificial \n on
                     * the end due to the presence of something like this:
                     * /foo$/ where we can match both "foo" and "foo\n" at the
                     * end of the string.  So we have to compare the end of the
                     * string first against the float_real without the \n and
                     * then against the full float_real with the string.  We
                     * have to watch out for cases where the string might be
                     * smaller than the float_real or the float_real without
                     * the \n. */
		    char *checkpos= strend - len;
		    DEBUG_OPTIMISE_r(
			PerlIO_printf(Perl_debug_log,
			    "%sChecking for float_real.%s\n",
			    PL_colors[4], PL_colors[5]));
		    if (checkpos + 1 < strbeg) {
                        /* can't match, even if we remove the trailing \n
                         * string is too short to match */
			DEBUG_EXECUTE_r(
			    PerlIO_printf(Perl_debug_log,
				"%sString shorter than required trailing substring, cannot match.%s\n",
				PL_colors[4], PL_colors[5]));
			goto phooey;
		    } else if (memEQ(checkpos + 1, little, len - 1)) {
                        /* can match, the end of the string matches without the
                         * "\n" */
			last = checkpos + 1;
		    } else if (checkpos < strbeg) {
                        /* cant match, string is too short when the "\n" is
                         * included */
			DEBUG_EXECUTE_r(
			    PerlIO_printf(Perl_debug_log,
				"%sString does not contain required trailing substring, cannot match.%s\n",
				PL_colors[4], PL_colors[5]));
			goto phooey;
		    } else if (!multiline) {
                        /* non multiline match, so compare with the "\n" at the
                         * end of the string */
			if (memEQ(checkpos, little, len)) {
			    last= checkpos;
			} else {
			    DEBUG_EXECUTE_r(
				PerlIO_printf(Perl_debug_log,
				    "%sString does not contain required trailing substring, cannot match.%s\n",
				    PL_colors[4], PL_colors[5]));
			    goto phooey;
			}
		    } else {
                        /* multiline match, so we have to search for a place
                         * where the full string is located */
			goto find_last;
		    }
	    } else {
		  find_last:
		    if (len)
			last = rninstr(s, strend, little, little + len);
		    else
			last = strend;	/* matching "$" */
	    }
	    if (!last) {
                /* at one point this block contained a comment which was
                 * probably incorrect, which said that this was a "should not
                 * happen" case.  Even if it was true when it was written I am
                 * pretty sure it is not anymore, so I have removed the comment
                 * and replaced it with this one. Yves */
		DEBUG_EXECUTE_r(
		    PerlIO_printf(Perl_debug_log,
			"%sString does not contain required substring, cannot match.%s\n",
                        PL_colors[4], PL_colors[5]
	            ));
		goto phooey;
	    }
	    dontbother = strend - last + prog->float_min_offset;
	}
	if (minlen && (dontbother < minlen))
	    dontbother = minlen - 1;
	strend -= dontbother; 		   /* this one's always in bytes! */
	/* We don't know much -- general case. */
	if (utf8_target) {
	    for (;;) {
		if (regtry(reginfo, &s))
		    goto got_it;
		if (s >= strend)
		    break;
		s += UTF8SKIP(s);
	    };
	}
	else {
	    do {
		if (regtry(reginfo, &s))
		    goto got_it;
	    } while (s++ < strend);
	}
    }

    /* Failure. */
    goto phooey;

  got_it:
    /* s/// doesn't like it if $& is earlier than where we asked it to
     * start searching (which can happen on something like /.\G/) */
    if (       (flags & REXEC_FAIL_ON_UNDERFLOW)
            && (prog->offs[0].start < stringarg - strbeg))
    {
        /* this should only be possible under \G */
        assert(prog->intflags & PREGf_GPOS_SEEN);
        DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log,
            "matched, but failing for REXEC_FAIL_ON_UNDERFLOW\n"));
        goto phooey;
    }

    DEBUG_BUFFERS_r(
	if (swap)
	    PerlIO_printf(Perl_debug_log,
		"rex=0x%"UVxf" freeing offs: 0x%"UVxf"\n",
		PTR2UV(prog),
		PTR2UV(swap)
	    );
    );
    Safefree(swap);

    /* clean up; this will trigger destructors that will free all slabs
     * above the current one, and cleanup the regmatch_info_aux
     * and regmatch_info_aux_eval sructs */

    LEAVE_SCOPE(oldsave);

    if (RXp_PAREN_NAMES(prog)) 
        (void)hv_iterinit(RXp_PAREN_NAMES(prog));

    /* make sure $`, $&, $', and $digit will work later */
    if ( !(flags & REXEC_NOT_FIRST) )
        S_reg_set_capture_string(aTHX_ rx,
                                    strbeg, reginfo->strend,
                                    sv, flags, utf8_target);

    return 1;

  phooey:
    DEBUG_EXECUTE_r(PerlIO_printf(Perl_debug_log, "%sMatch failed%s\n",
			  PL_colors[4], PL_colors[5]));

    /* clean up; this will trigger destructors that will free all slabs
     * above the current one, and cleanup the regmatch_info_aux
     * and regmatch_info_aux_eval sructs */

    LEAVE_SCOPE(oldsave);

    if (swap) {
        /* we failed :-( roll it back */
	DEBUG_BUFFERS_r(PerlIO_printf(Perl_debug_log,
	    "rex=0x%"UVxf" rolling back offs: freeing=0x%"UVxf" restoring=0x%"UVxf"\n",
	    PTR2UV(prog),
	    PTR2UV(prog->offs),
	    PTR2UV(swap)
	));
        Safefree(prog->offs);
        prog->offs = swap;
    }
    return 0;
}