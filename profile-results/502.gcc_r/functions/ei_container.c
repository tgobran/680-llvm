static inline VEC(edge,gc) *
ei_container (edge_iterator i)
{
  gcc_assert (i.container);
  return *i.container;
}
