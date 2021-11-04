/*
 * 'Away now, Shadowfax!  Run, greatheart, run as you have never run before! *  Now we are come to the lands where you were foaled, and every stone you
 *  know.  Run now!  Hope is in speed!'                    --Gandalf
 *
 *     [p.600 of _The Lord of the Rings_, III/xi: "The Palantír"]]
 */

int
Perl_runops_standard(pTHX)
{
    OP *op = PL_op;
    OP_ENTRY_PROBE(OP_NAME(op));
    while ((PL_op = op = op->op_ppaddr(aTHX))) {
        OP_ENTRY_PROBE(OP_NAME(op));
    }
    PERL_ASYNC_CHECK();

    TAINT_NOT;
    return 0;
}
