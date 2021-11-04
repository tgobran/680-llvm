void *
Perl_hv_common(pTHX_ HV *hv, SV *keysv, const char *key, STRLEN klen,
	       int flags, int action, SV *val, U32 hash)
{
    dVAR;
    XPVHV* xhv;
    HE *entry;
    HE **oentry;
    SV *sv;
    bool is_utf8;
    int masked_flags;
    const int return_svp = action & HV_FETCH_JUST_SV;
    HEK *keysv_hek = NULL;

    if (!hv)
	return NULL;
    if (SvTYPE(hv) == (svtype)SVTYPEMASK)
	return NULL;

    assert(SvTYPE(hv) == SVt_PVHV);

    if (SvSMAGICAL(hv) && SvGMAGICAL(hv) && !(action & HV_DISABLE_UVAR_XKEY)) {
	MAGIC* mg;
	if ((mg = mg_find((const SV *)hv, PERL_MAGIC_uvar))) {
	    struct ufuncs * const uf = (struct ufuncs *)mg->mg_ptr;
	    if (uf->uf_set == NULL) {
		SV* obj = mg->mg_obj;

		if (!keysv) {
		    keysv = newSVpvn_flags(key, klen, SVs_TEMP |
					   ((flags & HVhek_UTF8)
					    ? SVf_UTF8 : 0));
		}
		
		mg->mg_obj = keysv;         /* pass key */
		uf->uf_index = action;      /* pass action */
		magic_getuvar(MUTABLE_SV(hv), mg);
		keysv = mg->mg_obj;         /* may have changed */
		mg->mg_obj = obj;

		/* If the key may have changed, then we need to invalidate
		   any passed-in computed hash value.  */
		hash = 0;
	    }
	}
    }
    if (keysv) {
	if (flags & HVhek_FREEKEY)
	    Safefree(key);
	key = SvPV_const(keysv, klen);
	is_utf8 = (SvUTF8(keysv) != 0);
	if (SvIsCOW_shared_hash(keysv)) {
	    flags = HVhek_KEYCANONICAL | (is_utf8 ? HVhek_UTF8 : 0);
	} else {
	    flags = is_utf8 ? HVhek_UTF8 : 0;
	}
    } else {
	is_utf8 = ((flags & HVhek_UTF8) ? TRUE : FALSE);
    }

    if (action & HV_DELETE) {
	return (void *) hv_delete_common(hv, keysv, key, klen,
					 flags, action, hash);
    }

    xhv = (XPVHV*)SvANY(hv);
    if (SvMAGICAL(hv)) {
	if (SvRMAGICAL(hv) && !(action & (HV_FETCH_ISSTORE|HV_FETCH_ISEXISTS))) {
	    if (mg_find((const SV *)hv, PERL_MAGIC_tied)
		|| SvGMAGICAL((const SV *)hv))
	    {
		/* FIXME should be able to skimp on the HE/HEK here when
		   HV_FETCH_JUST_SV is true.  */
		if (!keysv) {
		    keysv = newSVpvn_utf8(key, klen, is_utf8);
  		} else {
		    keysv = newSVsv(keysv);
		}
                sv = sv_newmortal();
                mg_copy(MUTABLE_SV(hv), sv, (char *)keysv, HEf_SVKEY);

		/* grab a fake HE/HEK pair from the pool or make a new one */
		entry = PL_hv_fetch_ent_mh;
		if (entry)
		    PL_hv_fetch_ent_mh = HeNEXT(entry);
		else {
		    char *k;
		    entry = new_HE();
		    Newx(k, HEK_BASESIZE + sizeof(const SV *), char);
		    HeKEY_hek(entry) = (HEK*)k;
		}
		HeNEXT(entry) = NULL;
		HeSVKEY_set(entry, keysv);
		HeVAL(entry) = sv;
		sv_upgrade(sv, SVt_PVLV);
		LvTYPE(sv) = 'T';
		 /* so we can free entry when freeing sv */
		LvTARG(sv) = MUTABLE_SV(entry);

		/* XXX remove at some point? */
		if (flags & HVhek_FREEKEY)
		    Safefree(key);

		if (return_svp) {
		    return entry ? (void *) &HeVAL(entry) : NULL;
		}
		return (void *) entry;
	    }
#ifdef ENV_IS_CASELESS
	    else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
		U32 i;
		for (i = 0; i < klen; ++i)
		    if (isLOWER(key[i])) {
			/* Would be nice if we had a routine to do the
			   copy and upercase in a single pass through.  */
			const char * const nkey = strupr(savepvn(key,klen));
			/* Note that this fetch is for nkey (the uppercased
			   key) whereas the store is for key (the original)  */
			void *result = hv_common(hv, NULL, nkey, klen,
						 HVhek_FREEKEY, /* free nkey */
						 0 /* non-LVAL fetch */
						 | HV_DISABLE_UVAR_XKEY
						 | return_svp,
						 NULL /* no value */,
						 0 /* compute hash */);
			if (!result && (action & HV_FETCH_LVALUE)) {
			    /* This call will free key if necessary.
			       Do it this way to encourage compiler to tail
			       call optimise.  */
			    result = hv_common(hv, keysv, key, klen, flags,
					       HV_FETCH_ISSTORE
					       | HV_DISABLE_UVAR_XKEY
					       | return_svp,
					       newSV(0), hash);
			} else {
			    if (flags & HVhek_FREEKEY)
				Safefree(key);
			}
			return result;
		    }
	    }
#endif
	} /* ISFETCH */
	else if (SvRMAGICAL(hv) && (action & HV_FETCH_ISEXISTS)) {
	    if (mg_find((const SV *)hv, PERL_MAGIC_tied)
		|| SvGMAGICAL((const SV *)hv)) {
		/* I don't understand why hv_exists_ent has svret and sv,
		   whereas hv_exists only had one.  */
		SV * const svret = sv_newmortal();
		sv = sv_newmortal();

		if (keysv || is_utf8) {
		    if (!keysv) {
			keysv = newSVpvn_utf8(key, klen, TRUE);
		    } else {
			keysv = newSVsv(keysv);
		    }
		    mg_copy(MUTABLE_SV(hv), sv, (char *)sv_2mortal(keysv), HEf_SVKEY);
		} else {
		    mg_copy(MUTABLE_SV(hv), sv, key, klen);
		}
		if (flags & HVhek_FREEKEY)
		    Safefree(key);
		magic_existspack(svret, mg_find(sv, PERL_MAGIC_tiedelem));
		/* This cast somewhat evil, but I'm merely using NULL/
		   not NULL to return the boolean exists.
		   And I know hv is not NULL.  */
		return SvTRUE(svret) ? (void *)hv : NULL;
		}
#ifdef ENV_IS_CASELESS
	    else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
		/* XXX This code isn't UTF8 clean.  */
		char * const keysave = (char * const)key;
		/* Will need to free this, so set FREEKEY flag.  */
		key = savepvn(key,klen);
		key = (const char*)strupr((char*)key);
		is_utf8 = FALSE;
		hash = 0;
		keysv = 0;

		if (flags & HVhek_FREEKEY) {
		    Safefree(keysave);
		}
		flags |= HVhek_FREEKEY;
	    }
#endif
	} /* ISEXISTS */
	else if (action & HV_FETCH_ISSTORE) {
	    bool needs_copy;
	    bool needs_store;
	    hv_magic_check (hv, &needs_copy, &needs_store);
	    if (needs_copy) {
		const bool save_taint = TAINT_get;
		if (keysv || is_utf8) {
		    if (!keysv) {
			keysv = newSVpvn_utf8(key, klen, TRUE);
		    }
		    if (TAINTING_get)
			TAINT_set(SvTAINTED(keysv));
		    keysv = sv_2mortal(newSVsv(keysv));
		    mg_copy(MUTABLE_SV(hv), val, (char*)keysv, HEf_SVKEY);
		} else {
		    mg_copy(MUTABLE_SV(hv), val, key, klen);
		}

		TAINT_IF(save_taint);
#ifdef NO_TAINT_SUPPORT
                PERL_UNUSED_VAR(save_taint);
#endif
		if (!needs_store) {
		    if (flags & HVhek_FREEKEY)
			Safefree(key);
		    return NULL;
		}
#ifdef ENV_IS_CASELESS
		else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
		    /* XXX This code isn't UTF8 clean.  */
		    const char *keysave = key;
		    /* Will need to free this, so set FREEKEY flag.  */
		    key = savepvn(key,klen);
		    key = (const char*)strupr((char*)key);
		    is_utf8 = FALSE;
		    hash = 0;
		    keysv = 0;

		    if (flags & HVhek_FREEKEY) {
			Safefree(keysave);
		    }
		    flags |= HVhek_FREEKEY;
		}
#endif
	    }
	} /* ISSTORE */
    } /* SvMAGICAL */

    if (!HvARRAY(hv)) {
	if ((action & (HV_FETCH_LVALUE | HV_FETCH_ISSTORE))
#ifdef DYNAMIC_ENV_FETCH  /* if it's an %ENV lookup, we may get it on the fly */
		 || (SvRMAGICAL((const SV *)hv)
		     && mg_find((const SV *)hv, PERL_MAGIC_env))
#endif
								  ) {
	    char *array;
	    Newxz(array,
		 PERL_HV_ARRAY_ALLOC_BYTES(xhv->xhv_max+1 /* HvMAX(hv)+1 */),
		 char);
	    HvARRAY(hv) = (HE**)array;
	}
#ifdef DYNAMIC_ENV_FETCH
	else if (action & HV_FETCH_ISEXISTS) {
	    /* for an %ENV exists, if we do an insert it's by a recursive
	       store call, so avoid creating HvARRAY(hv) right now.  */
	}
#endif
	else {
	    /* XXX remove at some point? */
            if (flags & HVhek_FREEKEY)
                Safefree(key);

	    return NULL;
	}
    }

    if (is_utf8 && !(flags & HVhek_KEYCANONICAL)) {
	char * const keysave = (char *)key;
	key = (char*)bytes_from_utf8((U8*)key, &klen, &is_utf8);
        if (is_utf8)
	    flags |= HVhek_UTF8;
	else
	    flags &= ~HVhek_UTF8;
        if (key != keysave) {
	    if (flags & HVhek_FREEKEY)
		Safefree(keysave);
            flags |= HVhek_WASUTF8 | HVhek_FREEKEY;
	    /* If the caller calculated a hash, it was on the sequence of
	       octets that are the UTF-8 form. We've now changed the sequence
	       of octets stored to that of the equivalent byte representation,
	       so the hash we need is different.  */
	    hash = 0;
	}
    }

    if (keysv && (SvIsCOW_shared_hash(keysv))) {
        if (HvSHAREKEYS(hv))
            keysv_hek  = SvSHARED_HEK_FROM_PV(SvPVX_const(keysv));
        hash = SvSHARED_HASH(keysv);
    }
    else if (!hash)
        PERL_HASH(hash, key, klen);

    masked_flags = (flags & HVhek_MASK);

#ifdef DYNAMIC_ENV_FETCH
    if (!HvARRAY(hv)) entry = NULL;
    else
#endif
    {
	entry = (HvARRAY(hv))[hash & (I32) HvMAX(hv)];
    }

    if (!entry)
        goto not_found;

    if (keysv_hek) {
        /* keysv is actually a HEK in disguise, so we can match just by
         * comparing the HEK pointers in the HE chain. There is a slight
         * caveat: on something like "\x80", which has both plain and utf8
         * representations, perl's hashes do encoding-insensitive lookups,
         * but preserve the encoding of the stored key. Thus a particular
         * key could map to two different HEKs in PL_strtab. We only
         * conclude 'not found' if all the flags are the same; otherwise
         * we fall back to a full search (this should only happen in rare
         * cases).
         */
        int keysv_flags = HEK_FLAGS(keysv_hek);
        HE  *orig_entry = entry;

        for (; entry; entry = HeNEXT(entry)) {
            HEK *hek = HeKEY_hek(entry);
            if (hek == keysv_hek)
                goto found;
            if (HEK_FLAGS(hek) != keysv_flags)
                break; /* need to do full match */
        }
        if (!entry)
            goto not_found;
        /* failed on shortcut - do full search loop */
        entry = orig_entry;
    }

    for (; entry; entry = HeNEXT(entry)) {
	if (HeHASH(entry) != hash)		/* strings can't be equal */
	    continue;
	if (HeKLEN(entry) != (I32)klen)
	    continue;
	if (memNE(HeKEY(entry),key,klen))	/* is this it? */
	    continue;
	if ((HeKFLAGS(entry) ^ masked_flags) & HVhek_UTF8)
	    continue;

      found:
        if (action & (HV_FETCH_LVALUE|HV_FETCH_ISSTORE)) {
	    if (HeKFLAGS(entry) != masked_flags) {
		/* We match if HVhek_UTF8 bit in our flags and hash key's
		   match.  But if entry was set previously with HVhek_WASUTF8
		   and key now doesn't (or vice versa) then we should change
		   the key's flag, as this is assignment.  */
		if (HvSHAREKEYS(hv)) {
		    /* Need to swap the key we have for a key with the flags we
		       need. As keys are shared we can't just write to the
		       flag, so we share the new one, unshare the old one.  */
		    HEK * const new_hek = share_hek_flags(key, klen, hash,
						   masked_flags);
		    unshare_hek (HeKEY_hek(entry));
		    HeKEY_hek(entry) = new_hek;
		}
		else if (hv == PL_strtab) {
		    /* PL_strtab is usually the only hash without HvSHAREKEYS,
		       so putting this test here is cheap  */
		    if (flags & HVhek_FREEKEY)
			Safefree(key);
		    Perl_croak(aTHX_ S_strtab_error,
			       action & HV_FETCH_LVALUE ? "fetch" : "store");
		}
		else
		    HeKFLAGS(entry) = masked_flags;
		if (masked_flags & HVhek_ENABLEHVKFLAGS)
		    HvHASKFLAGS_on(hv);
	    }
	    if (HeVAL(entry) == &PL_sv_placeholder) {
		/* yes, can store into placeholder slot */
		if (action & HV_FETCH_LVALUE) {
		    if (SvMAGICAL(hv)) {
			/* This preserves behaviour with the old hv_fetch
			   implementation which at this point would bail out
			   with a break; (at "if we find a placeholder, we
			   pretend we haven't found anything")

			   That break mean that if a placeholder were found, it
			   caused a call into hv_store, which in turn would
			   check magic, and if there is no magic end up pretty
			   much back at this point (in hv_store's code).  */
			break;
		    }
		    /* LVAL fetch which actually needs a store.  */
		    val = newSV(0);
		    HvPLACEHOLDERS(hv)--;
		} else {
		    /* store */
		    if (val != &PL_sv_placeholder)
			HvPLACEHOLDERS(hv)--;
		}
		HeVAL(entry) = val;
	    } else if (action & HV_FETCH_ISSTORE) {
		SvREFCNT_dec(HeVAL(entry));
		HeVAL(entry) = val;
	    }
	} else if (HeVAL(entry) == &PL_sv_placeholder) {
	    /* if we find a placeholder, we pretend we haven't found
	       anything */
	    break;
	}
	if (flags & HVhek_FREEKEY)
	    Safefree(key);
	if (return_svp) {
	    return entry ? (void *) &HeVAL(entry) : NULL;
	}
	return entry;
    }

  not_found:
#ifdef DYNAMIC_ENV_FETCH  /* %ENV lookup?  If so, try to fetch the value now */
    if (!(action & HV_FETCH_ISSTORE) 
	&& SvRMAGICAL((const SV *)hv)
	&& mg_find((const SV *)hv, PERL_MAGIC_env)) {
	unsigned long len;
	const char * const env = PerlEnv_ENVgetenv_len(key,&len);
	if (env) {
	    sv = newSVpvn(env,len);
	    SvTAINTED_on(sv);
	    return hv_common(hv, keysv, key, klen, flags,
			     HV_FETCH_ISSTORE|HV_DISABLE_UVAR_XKEY|return_svp,
			     sv, hash);
	}
    }
#endif

    if (!entry && SvREADONLY(hv) && !(action & HV_FETCH_ISEXISTS)) {
	hv_notallowed(flags, key, klen,
			"Attempt to access disallowed key '%"SVf"' in"
			" a restricted hash");
    }
    if (!(action & (HV_FETCH_LVALUE|HV_FETCH_ISSTORE))) {
	/* Not doing some form of store, so return failure.  */
	if (flags & HVhek_FREEKEY)
	    Safefree(key);
	return NULL;
    }
    if (action & HV_FETCH_LVALUE) {
	val = action & HV_FETCH_EMPTY_HE ? NULL : newSV(0);
	if (SvMAGICAL(hv)) {
	    /* At this point the old hv_fetch code would call to hv_store,
	       which in turn might do some tied magic. So we need to make that
	       magic check happen.  */
	    /* gonna assign to this, so it better be there */
	    /* If a fetch-as-store fails on the fetch, then the action is to
	       recurse once into "hv_store". If we didn't do this, then that
	       recursive call would call the key conversion routine again.
	       However, as we replace the original key with the converted
	       key, this would result in a double conversion, which would show
	       up as a bug if the conversion routine is not idempotent.
	       Hence the use of HV_DISABLE_UVAR_XKEY.  */
	    return hv_common(hv, keysv, key, klen, flags,
			     HV_FETCH_ISSTORE|HV_DISABLE_UVAR_XKEY|return_svp,
			     val, hash);
	    /* XXX Surely that could leak if the fetch-was-store fails?
	       Just like the hv_fetch.  */
	}
    }

    /* Welcome to hv_store...  */

    if (!HvARRAY(hv)) {
	/* Not sure if we can get here.  I think the only case of oentry being
	   NULL is for %ENV with dynamic env fetch.  But that should disappear
	   with magic in the previous code.  */
	char *array;
	Newxz(array,
	     PERL_HV_ARRAY_ALLOC_BYTES(xhv->xhv_max+1 /* HvMAX(hv)+1 */),
	     char);
	HvARRAY(hv) = (HE**)array;
    }

    oentry = &(HvARRAY(hv))[hash & (I32) xhv->xhv_max];

    entry = new_HE();
    /* share_hek_flags will do the free for us.  This might be considered
       bad API design.  */
    if (HvSHAREKEYS(hv))
	HeKEY_hek(entry) = share_hek_flags(key, klen, hash, flags);
    else if (hv == PL_strtab) {
	/* PL_strtab is usually the only hash without HvSHAREKEYS, so putting
	   this test here is cheap  */
	if (flags & HVhek_FREEKEY)
	    Safefree(key);
	Perl_croak(aTHX_ S_strtab_error,
		   action & HV_FETCH_LVALUE ? "fetch" : "store");
    }
    else                                       /* gotta do the real thing */
	HeKEY_hek(entry) = save_hek_flags(key, klen, hash, flags);
    HeVAL(entry) = val;

    if (!*oentry && SvOOK(hv)) {
        /* initial entry, and aux struct present.  */
        struct xpvhv_aux *const aux = HvAUX(hv);
        if (aux->xhv_fill_lazy)
            ++aux->xhv_fill_lazy;
    }

#ifdef PERL_HASH_RANDOMIZE_KEYS
    /* This logic semi-randomizes the insert order in a bucket.
     * Either we insert into the top, or the slot below the top,
     * making it harder to see if there is a collision. We also
     * reset the iterator randomizer if there is one.
     */
    if ( *oentry && PL_HASH_RAND_BITS_ENABLED) {
        PL_hash_rand_bits++;
        PL_hash_rand_bits= ROTL_UV(PL_hash_rand_bits,1);
        if ( PL_hash_rand_bits & 1 ) {
            HeNEXT(entry) = HeNEXT(*oentry);
            HeNEXT(*oentry) = entry;
        } else {
            HeNEXT(entry) = *oentry;
            *oentry = entry;
        }
    } else
#endif
    {
        HeNEXT(entry) = *oentry;
        *oentry = entry;
    }
#ifdef PERL_HASH_RANDOMIZE_KEYS
    if (SvOOK(hv)) {
        /* Currently this makes various tests warn in annoying ways.
         * So Silenced for now. - Yves | bogus end of comment =>* /
        if (HvAUX(hv)->xhv_riter != -1) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "[TESTING] Inserting into a hash during each() traversal results in undefined behavior"
                             pTHX__FORMAT
                             pTHX__VALUE);
        }
        */
        if (PL_HASH_RAND_BITS_ENABLED) {
            if (PL_HASH_RAND_BITS_ENABLED == 1)
                PL_hash_rand_bits += (PTRV)entry + 1;  /* we don't bother to use ptr_hash here */
            PL_hash_rand_bits= ROTL_UV(PL_hash_rand_bits,1);
        }
        HvAUX(hv)->xhv_rand= (U32)PL_hash_rand_bits;
    }
#endif

    if (val == &PL_sv_placeholder)
	HvPLACEHOLDERS(hv)++;
    if (masked_flags & HVhek_ENABLEHVKFLAGS)
	HvHASKFLAGS_on(hv);

    xhv->xhv_keys++; /* HvTOTALKEYS(hv)++ */
    if ( DO_HSPLIT(xhv) ) {
        const STRLEN oldsize = xhv->xhv_max + 1;
        const U32 items = (U32)HvPLACEHOLDERS_get(hv);

        if (items /* hash has placeholders  */
            && !SvREADONLY(hv) /* but is not a restricted hash */) {
            /* If this hash previously was a "restricted hash" and had
               placeholders, but the "restricted" flag has been turned off,
               then the placeholders no longer serve any useful purpose.
               However, they have the downsides of taking up RAM, and adding
               extra steps when finding used values. It's safe to clear them
               at this point, even though Storable rebuilds restricted hashes by
               putting in all the placeholders (first) before turning on the
               readonly flag, because Storable always pre-splits the hash.
               If we're lucky, then we may clear sufficient placeholders to
               avoid needing to split the hash at all.  */
            clear_placeholders(hv, items);
            if (DO_HSPLIT(xhv))
                hsplit(hv, oldsize, oldsize * 2);
        } else
            hsplit(hv, oldsize, oldsize * 2);
    }

    if (return_svp) {
	return entry ? (void *) &HeVAL(entry) : NULL;
    }
    return (void *) entry;
}