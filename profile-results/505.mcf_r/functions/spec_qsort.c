void
spec_qsort(void *a, size_t n, size_t es, cmp_t *cmp)
{
        char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
        size_t d, r;
        int cmp_result;
        int swaptype_long, swaptype_int, swap_cnt;

loop:   SWAPINIT(long, a, es);
        SWAPINIT(int, a, es);
        swap_cnt = 0;
        if (n < 7) {
                for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
                        for (pl = pm; 
                             pl > (char *)a && cmp(pl - es, pl) > 0;
                             pl -= es)
                                swap(pl, pl - es);
                return;
        }
        pm = (char *)a + (n / 2) * es;
        if (n > 7) {
                pl = (char *)a;
                pn = (char *)a + (n - 1) * es;
                if (n > 40) {
                        d = (n / 8) * es;
                        pl = med3(pl, pl + d, pl + 2 * d, cmp);
                        pm = med3(pm - d, pm, pm + d, cmp);
                        pn = med3(pn - 2 * d, pn - d, pn, cmp);
                }
                pm = med3(pl, pm, pn, cmp);
        }
        swap(a, pm);
        pa = pb = (char *)a + es;

        pc = pd = (char *)a + (n - 1) * es;
        for (;;) {
                while (pb <= pc && (cmp_result = cmp(pb, a)) <= 0) {
                        if (cmp_result == 0) {
                                swap_cnt = 1;
                                swap(pa, pb);
                                pa += es;
                        }
                        pb += es;
                }
                while (pb <= pc && (cmp_result = cmp(pc, a)) >= 0) {
                        if (cmp_result == 0) {
                                swap_cnt = 1;
                                swap(pc, pd);
                                pd -= es;
                        }
                        pc -= es;
                }
                if (pb > pc)
                        break;
                swap(pb, pc);
                swap_cnt = 1;
                pb += es;
                pc -= es;
        }
        if (swap_cnt == 0) {  /* Switch to insertion sort */
                for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
                        for (pl = pm; 
                             pl > (char *)a && cmp(pl - es, pl) > 0;
                             pl -= es)
                                swap(pl, pl - es);
                return;
        }

        pn = (char *)a + n * es;
        r = MIN(pa - (char *)a, pb - pa);
        vecswap(a, pb - r, r);
        r = MIN(pd - pc, pn - pd - (int64_t)es);
        vecswap(pb, pn - r, r);
        if ((r = pb - pa) > es)
                spec_qsort(a, r / es, es, cmp);
        if ((r = pd - pc) > es) {
                /* Iterate rather than recurse to save stack space */
                a = pn - r;
                n = r / es;
                goto loop;
        }
/*              spec_qsort(pn - r, r / es, es, cmp);*/
}
