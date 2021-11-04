void
Perl_leave_scope(pTHX_ I32 base)
{
    /* Localise the effects of the TAINT_NOT inside the loop.  */
    bool was = TAINT_get;

    I32 i;
    SV *sv;

    ANY arg0, arg1, arg2;

    /* these initialisations are logically unnecessary, but they shut up
     * spurious 'may be used uninitialized' compiler warnings */
    arg0.any_ptr = NULL;
    arg1.any_ptr = NULL;
    arg2.any_ptr = NULL;

    if (UNLIKELY(base < -1))
	Perl_croak(aTHX_ "panic: corrupt saved stack index %ld", (long) base);
    DEBUG_l(Perl_deb(aTHX_ "savestack: releasing items %ld -> %ld\n",
			(long)PL_savestack_ix, (long)base));
    while (PL_savestack_ix > base) {
	UV uv;
	U8 type;

        SV *refsv;
        SV **svp;

	TAINT_NOT;

        {
            I32 ix = PL_savestack_ix - 1;
            ANY *p = &PL_savestack[ix];
            uv = p->any_uv;
            type = (U8)uv & SAVE_MASK;
            if (type > SAVEt_ARG0_MAX) {
                ANY *p0 = p;
                arg0 = *--p;
                if (type > SAVEt_ARG1_MAX) {
                    arg1 = *--p;
                    if (type > SAVEt_ARG2_MAX) {
                        arg2 = *--p;
                    }
                }
                ix -= (p0 - p);
            }
            PL_savestack_ix = ix;
        }

	switch (type) {
	case SAVEt_ITEM:			/* normal string */
	    sv_replace(ARG1_SV, ARG0_SV);
            if (UNLIKELY(SvSMAGICAL(ARG1_SV))) {
                PL_localizing = 2;
                mg_set(ARG1_SV);
                PL_localizing = 0;
            }
	    break;

	    /* This would be a mathom, but Perl_save_svref() calls a static
	       function, S_save_scalar_at(), so has to stay in this file.  */
	case SAVEt_SVREF:			/* scalar reference */
	    svp = ARG1_SVP;
	    refsv = NULL; /* what to refcnt_dec */
	    goto restore_sv;

	case SAVEt_SV:				/* scalar reference */
	    svp = &GvSV(ARG1_GV);
	    refsv = ARG1_SV; /* what to refcnt_dec */
	restore_sv:
        {
	    SV * const sv = *svp;
	    *svp = ARG0_SV;
	    SvREFCNT_dec(sv);
            if (UNLIKELY(SvSMAGICAL(ARG0_SV))) {
                PL_localizing = 2;
                mg_set(ARG0_SV);
                PL_localizing = 0;
            }
	    SvREFCNT_dec_NN(ARG0_SV);
	    SvREFCNT_dec(refsv);
	    break;
        }
	case SAVEt_GENERIC_PVREF:		/* generic pv */
	    if (*ARG0_PVP != ARG1_PV) {
		Safefree(*ARG0_PVP);
		*ARG0_PVP = ARG1_PV;
	    }
	    break;
	case SAVEt_SHARED_PVREF:		/* shared pv */
	    if (*ARG1_PVP != ARG0_PV) {
#ifdef NETWARE
		PerlMem_free(*ARG1_PVP);
#else
		PerlMemShared_free(*ARG1_PVP);
#endif
		*ARG1_PVP = ARG0_PV;
	    }
	    break;
	case SAVEt_GVSV:			/* scalar slot in GV */
	    svp = &GvSV(ARG1_GV);
	    goto restore_svp;
	case SAVEt_GENERIC_SVREF:		/* generic sv */
            svp = ARG1_SVP;
	restore_svp:
        {
	    SV * const sv = *svp;
	    *svp = ARG0_SV;
	    SvREFCNT_dec(sv);
	    SvREFCNT_dec(ARG0_SV);
	    break;
        }
	case SAVEt_GVSLOT:			/* any slot in GV */
        {
            HV *const hv = GvSTASH(ARG2_GV);
	    svp = ARG1_SVP;
	    if (hv && HvENAME(hv) && (
		    (ARG0_SV && SvTYPE(ARG0_SV) == SVt_PVCV)
		 || (*svp && SvTYPE(*svp) == SVt_PVCV)
	       ))
	    {
		if ((char *)svp < (char *)GvGP(ARG2_GV)
		 || (char *)svp > (char *)GvGP(ARG2_GV) + sizeof(struct gp)
		 || GvREFCNT(ARG2_GV) > 2) /* "> 2" to ignore savestack's ref */
		    PL_sub_generation++;
		else mro_method_changed_in(hv);
	    }
	    goto restore_svp;
        }
	case SAVEt_AV:				/* array reference */
	    SvREFCNT_dec(GvAV(ARG1_GV));
	    GvAV(ARG1_GV) = ARG0_AV;
            if (UNLIKELY(SvSMAGICAL(ARG0_SV))) {
                PL_localizing = 2;
                mg_set(ARG0_SV);
                PL_localizing = 0;
            }
	    SvREFCNT_dec_NN(ARG1_GV);
	    break;
	case SAVEt_HV:				/* hash reference */
	    SvREFCNT_dec(GvHV(ARG1_GV));
	    GvHV(ARG1_GV) = ARG0_HV;
            if (UNLIKELY(SvSMAGICAL(ARG0_SV))) {
                PL_localizing = 2;
                mg_set(ARG0_SV);
                PL_localizing = 0;
            }
	    SvREFCNT_dec_NN(ARG1_GV);
	    break;
	case SAVEt_INT_SMALL:
	    *(int*)ARG0_PTR = (int)(uv >> SAVE_TIGHT_SHIFT);
	    break;
	case SAVEt_INT:				/* int reference */
	    *(int*)ARG0_PTR = (int)ARG1_I32;
	    break;
	case SAVEt_STRLEN:			/* STRLEN/size_t ref */
	    *(STRLEN*)ARG0_PTR = (STRLEN)arg1.any_iv;
	    break;
	case SAVEt_BOOL:			/* bool reference */
	    *(bool*)ARG0_PTR = cBOOL(uv >> 8);
#ifdef NO_TAINT_SUPPORT
            PERL_UNUSED_VAR(was);
#else
	    if (UNLIKELY(ARG0_PTR == &(TAINT_get))) {
		/* If we don't update <was>, to reflect what was saved on the
		 * stack for PL_tainted, then we will overwrite this attempt to
		 * restore it when we exit this routine.  Note that this won't
		 * work if this value was saved in a wider-than necessary type,
		 * such as I32 */
		was = *(bool*)ARG0_PTR;
	    }
#endif
	    break;
	case SAVEt_I32_SMALL:
	    *(I32*)ARG0_PTR = (I32)(uv >> SAVE_TIGHT_SHIFT);
	    break;
	case SAVEt_I32:				/* I32 reference */
#ifdef PERL_DEBUG_READONLY_OPS
            if (*(I32*)ARG0_PTR != ARG1_I32)
#endif
                *(I32*)ARG0_PTR = ARG1_I32;
	    break;
	case SAVEt_SPTR:			/* SV* reference */
	    *(SV**)(ARG0_PTR)= ARG1_SV;
	    break;
	case SAVEt_VPTR:			/* random* reference */
	case SAVEt_PPTR:			/* char* reference */
	    *ARG0_PVP = ARG1_PV;
	    break;
	case SAVEt_HPTR:			/* HV* reference */
	    *(HV**)ARG0_PTR = MUTABLE_HV(ARG1_PTR);
	    break;
	case SAVEt_APTR:			/* AV* reference */
	    *(AV**)ARG0_PTR = ARG1_AV;
	    break;
	case SAVEt_GP:				/* scalar reference */
        {
            HV *hv;
            /* possibly taking a method out of circulation */	
	    const bool had_method = !!GvCVu(ARG1_GV);
	    gp_free(ARG1_GV);
	    GvGP_set(ARG1_GV, (GP*)ARG0_PTR);
	    if ((hv=GvSTASH(ARG1_GV)) && HvENAME_get(hv)) {
	        if (   GvNAMELEN(ARG1_GV) == 3
                    && strnEQ(GvNAME(ARG1_GV), "ISA", 3)
                )
	            mro_isa_changed_in(hv);
                else if (had_method || GvCVu(ARG1_GV))
                    /* putting a method back into circulation ("local")*/	
                    gv_method_changed(ARG1_GV);
	    }
	    SvREFCNT_dec_NN(ARG1_GV);
	    break;
        }
	case SAVEt_FREESV:
	    SvREFCNT_dec(ARG0_SV);
	    break;
	case SAVEt_FREEPADNAME:
	    PadnameREFCNT_dec((PADNAME *)ARG0_PTR);
	    break;
	case SAVEt_FREECOPHH:
	    cophh_free((COPHH *)ARG0_PTR);
	    break;
	case SAVEt_MORTALIZESV:
	    sv_2mortal(ARG0_SV);
	    break;
	case SAVEt_FREEOP:
	    ASSERT_CURPAD_LEGAL("SAVEt_FREEOP");
	    op_free((OP*)ARG0_PTR);
	    break;
	case SAVEt_FREEPV:
	    Safefree(ARG0_PTR);
	    break;

        case SAVEt_CLEARPADRANGE:
            i = (I32)((uv >> SAVE_TIGHT_SHIFT) & OPpPADRANGE_COUNTMASK);
	    svp = &PL_curpad[uv >>
                    (OPpPADRANGE_COUNTSHIFT + SAVE_TIGHT_SHIFT)] + i - 1;
            goto clearsv;
	case SAVEt_CLEARSV:
	    svp = &PL_curpad[uv >> SAVE_TIGHT_SHIFT];
            i = 1;
          clearsv:
            for (; i; i--, svp--) {
                sv = *svp;

                DEBUG_Xv(PerlIO_printf(Perl_debug_log,
             "Pad 0x%"UVxf"[0x%"UVxf"] clearsv: %ld sv=0x%"UVxf"<%"IVdf"> %s\n",
                    PTR2UV(PL_comppad), PTR2UV(PL_curpad),
                    (long)(svp-PL_curpad), PTR2UV(sv), (IV)SvREFCNT(sv),
                    (SvREFCNT(sv) <= 1 && !SvOBJECT(sv)) ? "clear" : "abandon"
                ));

                /* Can clear pad variable in place? */
                if (SvREFCNT(sv) == 1 && !SvOBJECT(sv)) {

                    /* these flags are the union of all the relevant flags
                     * in the individual conditions within */
                    if (UNLIKELY(SvFLAGS(sv) & (
                            SVf_READONLY|SVf_PROTECT /*for SvREADONLY_off*/
                          | (SVs_GMG|SVs_SMG|SVs_RMG) /* SvMAGICAL() */
                          | SVf_OOK
                          | SVf_THINKFIRST)))
                    {
                        /* if a my variable that was made readonly is
                         * going out of scope, we want to remove the
                         * readonlyness so that it can go out of scope
                         * quietly
                         */
                        if (SvREADONLY(sv))
                            SvREADONLY_off(sv);

                        if (SvOOK(sv)) { /* OOK or HvAUX */
                            if (SvTYPE(sv) == SVt_PVHV)
                                Perl_hv_kill_backrefs(aTHX_ MUTABLE_HV(sv));
                            else
                                sv_backoff(sv);
                        }

                        if (SvMAGICAL(sv)) {
                            /* note that backrefs (either in HvAUX or magic)
                             * must be removed before other magic */
                            sv_unmagic(sv, PERL_MAGIC_backref);
                            if (SvTYPE(sv) != SVt_PVCV)
                                mg_free(sv);
                        }
                        if (SvTHINKFIRST(sv))
                            sv_force_normal_flags(sv, SV_IMMEDIATE_UNREF
                                                     |SV_COW_DROP_PV);

                    }
                    switch (SvTYPE(sv)) {
                    case SVt_NULL:
                        break;
                    case SVt_PVAV:
                        av_clear(MUTABLE_AV(sv));
                        break;
                    case SVt_PVHV:
                        hv_clear(MUTABLE_HV(sv));
                        break;
                    case SVt_PVCV:
                    {
                        HEK *hek =
			      CvNAMED(sv)
				? CvNAME_HEK((CV *)sv)
				: GvNAME_HEK(CvGV(sv));
                        assert(hek);
                        (void)share_hek_hek(hek);
                        cv_undef((CV *)sv);
                        CvNAME_HEK_set(sv, hek);
                        CvLEXICAL_on(sv);
                        break;
                    }
                    default:
                        /* This looks odd, but these two macros are for use in
                           expressions and finish with a trailing comma, so
                           adding a ; after them would be wrong. */
                        assert_not_ROK(sv)
                        assert_not_glob(sv)
                        SvFLAGS(sv) &=~ (SVf_OK|SVf_IVisUV|SVf_UTF8);
                        break;
                    }
                    SvPADTMP_off(sv);
                    SvPADSTALE_on(sv); /* mark as no longer live */
                }
                else {	/* Someone has a claim on this, so abandon it. */
                    switch (SvTYPE(sv)) {	/* Console ourselves with a new value */
                    case SVt_PVAV:	*svp = MUTABLE_SV(newAV());	break;
                    case SVt_PVHV:	*svp = MUTABLE_SV(newHV());	break;
                    case SVt_PVCV:
                    {
                        HEK * const hek = CvNAMED(sv)
                                             ? CvNAME_HEK((CV *)sv)
                                             : GvNAME_HEK(CvGV(sv));

                        /* Create a stub */
                        *svp = newSV_type(SVt_PVCV);

                        /* Share name */
                        CvNAME_HEK_set(*svp,
                                       share_hek_hek(hek));
                        CvLEXICAL_on(*svp);
                        break;
                    }
                    default:	*svp = newSV(0);		break;
                    }
                    SvREFCNT_dec_NN(sv); /* Cast current value to the winds. */
                    /* preserve pad nature, but also mark as not live
                     * for any closure capturing */
                    SvFLAGS(*svp) |= SVs_PADSTALE;
                }
            }
	    break;
	case SAVEt_DELETE:
	    (void)hv_delete(ARG0_HV, ARG2_PV, ARG1_I32, G_DISCARD);
	    SvREFCNT_dec(ARG0_HV);
	    Safefree(arg2.any_ptr);
	    break;
	case SAVEt_ADELETE:
	    (void)av_delete(ARG0_AV, arg1.any_iv, G_DISCARD);
	    SvREFCNT_dec(ARG0_AV);
	    break;
	case SAVEt_DESTRUCTOR_X:
	    (*arg1.any_dxptr)(aTHX_ ARG0_PTR);
	    break;
	case SAVEt_REGCONTEXT:
	    /* regexp must have croaked */
	case SAVEt_ALLOC:
	    PL_savestack_ix -= uv >> SAVE_TIGHT_SHIFT;
	    break;
	case SAVEt_STACK_POS:		/* Position on Perl stack */
	    PL_stack_sp = PL_stack_base + arg0.any_i32;
	    break;
	case SAVEt_AELEM:		/* array element */
	    svp = av_fetch(ARG2_AV, arg1.any_iv, 1);
	    if (UNLIKELY(!AvREAL(ARG2_AV) && AvREIFY(ARG2_AV))) /* undo reify guard */
		SvREFCNT_dec(ARG0_SV);
	    if (LIKELY(svp)) {
		SV * const sv = *svp;
		if (LIKELY(sv && sv != &PL_sv_undef)) {
		    if (UNLIKELY(SvTIED_mg((const SV *)ARG2_AV, PERL_MAGIC_tied)))
			SvREFCNT_inc_void_NN(sv);
                    refsv = ARG2_SV;
		    goto restore_sv;
		}
	    }
	    SvREFCNT_dec(ARG2_AV);
	    SvREFCNT_dec(ARG0_SV);
	    break;
	case SAVEt_HELEM:		/* hash element */
        {
	    HE * const he = hv_fetch_ent(ARG2_HV, ARG1_SV, 1, 0);
	    SvREFCNT_dec(ARG1_SV);
	    if (LIKELY(he)) {
		const SV * const oval = HeVAL(he);
		if (LIKELY(oval && oval != &PL_sv_undef)) {
		    svp = &HeVAL(he);
		    if (UNLIKELY(SvTIED_mg((const SV *)ARG2_HV, PERL_MAGIC_tied)))
			SvREFCNT_inc_void(*svp);
		    refsv = ARG2_SV; /* what to refcnt_dec */
		    goto restore_sv;
		}
	    }
	    SvREFCNT_dec(ARG2_HV);
	    SvREFCNT_dec(ARG0_SV);
	    break;
        }
	case SAVEt_OP:
	    PL_op = (OP*)ARG0_PTR;
	    break;
	case SAVEt_HINTS:
	    if ((PL_hints & HINT_LOCALIZE_HH)) {
	      while (GvHV(PL_hintgv)) {
		HV *hv = GvHV(PL_hintgv);
		GvHV(PL_hintgv) = NULL;
		SvREFCNT_dec(MUTABLE_SV(hv));
	      }
	    }
	    cophh_free(CopHINTHASH_get(&PL_compiling));
	    CopHINTHASH_set(&PL_compiling, (COPHH*)ARG0_PTR);
	    *(I32*)&PL_hints = ARG1_I32;
	    if (PL_hints & HINT_LOCALIZE_HH) {
		SvREFCNT_dec(MUTABLE_SV(GvHV(PL_hintgv)));
		GvHV(PL_hintgv) = MUTABLE_HV(SSPOPPTR);
	    }
	    if (!GvHV(PL_hintgv)) {
		/* Need to add a new one manually, else rv2hv can
		   add one via GvHVn and it won't have the magic set.  */
		HV *const hv = newHV();
		hv_magic(hv, NULL, PERL_MAGIC_hints);
		GvHV(PL_hintgv) = hv;
	    }
	    assert(GvHV(PL_hintgv));
	    break;
	case SAVEt_COMPPAD:
	    PL_comppad = (PAD*)ARG0_PTR;
	    if (LIKELY(PL_comppad))
		PL_curpad = AvARRAY(PL_comppad);
	    else
		PL_curpad = NULL;
	    break;
	case SAVEt_PADSV_AND_MORTALIZE:
	    {
		SV **svp;
		assert (ARG1_PTR);
		svp = AvARRAY((PAD*)ARG1_PTR) + (PADOFFSET)arg0.any_uv;
		/* This mortalizing used to be done by POPLOOP() via itersave.
		   But as we have all the information here, we can do it here,
		   save even having to have itersave in the struct.  */
		sv_2mortal(*svp);
		*svp = ARG2_SV;
	    }
	    break;
	case SAVEt_SAVESWITCHSTACK:
	    {
		dSP;
		SWITCHSTACK(ARG0_AV, ARG1_AV);
		PL_curstackinfo->si_stack = ARG1_AV;
	    }
	    break;
	case SAVEt_SET_SVFLAGS:
            SvFLAGS(ARG2_SV) &= ~((U32)ARG1_I32);
            SvFLAGS(ARG2_SV) |= (U32)ARG0_I32;
	    break;

	    /* These are only saved in mathoms.c */
	case SAVEt_NSTAB:
	    (void)sv_clear(ARG0_SV);
	    break;
	case SAVEt_LONG:			/* long reference */
	    *(long*)ARG0_PTR = arg1.any_long;
	    break;
	case SAVEt_IV:				/* IV reference */
	    *(IV*)ARG0_PTR = arg1.any_iv;
	    break;

	case SAVEt_I16:				/* I16 reference */
	    *(I16*)ARG0_PTR = (I16)(uv >> 8);
	    break;
	case SAVEt_I8:				/* I8 reference */
	    *(I8*)ARG0_PTR = (I8)(uv >> 8);
	    break;
	case SAVEt_DESTRUCTOR:
	    (*arg1.any_dptr)(ARG0_PTR);
	    break;
	case SAVEt_COMPILE_WARNINGS:
	    if (!specialWARN(PL_compiling.cop_warnings))
		PerlMemShared_free(PL_compiling.cop_warnings);

	    PL_compiling.cop_warnings = (STRLEN*)ARG0_PTR;
	    break;
	case SAVEt_PARSER:
	    parser_free((yy_parser *) ARG0_PTR);
	    break;
	case SAVEt_READONLY_OFF:
	    SvREADONLY_off(ARG0_SV);
	    break;
	case SAVEt_GP_ALIASED_SV: {
	    /* The GP may have been abandoned, leaving the savestack with
	       the only remaining reference to it.  */
	    GP * const gp = (GP *)ARG0_PTR;
	    if (gp->gp_refcnt == 1) {
		GV * const gv = (GV *)sv_2mortal(newSV_type(SVt_PVGV));
		isGV_with_GP_on(gv);
		GvGP_set(gv,gp);
		gp_free(gv);
		isGV_with_GP_off(gv);
	    }
	    else {
		gp->gp_refcnt--;
		if (uv >> 8) gp->gp_flags |=  GPf_ALIASED_SV;
		else	     gp->gp_flags &= ~GPf_ALIASED_SV;
	    }
	    break;
	}
	default:
	    Perl_croak(aTHX_ "panic: leave_scope inconsistency %u", type);
	}
    }

    TAINT_set(was);
}