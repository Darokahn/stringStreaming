// TODO:
// an implementation of a line printer which is specialized toward strings (but more generally, streams which can be printed to and also repointed) and applies ANSI escape cursor controls as they appear.
// if an ANSI code cannot be applied to plain text (color for example), it will simply be removed from the stream.
// provides a `stream` function but also a `repoint` function so that the head of the iterator can be moved
// cursor codes treat text-space as a 2D grid, which requires significant bookkeeping as well as memory-moving to reflect operations in a static buffer.
// a heirarchy of compatible pieces will be defined; Top-level interceptor defers cursor control to a cursor-mapping device; cursor-mapping device wraps underlying stream buffer and can interpret cursor-mapping however it likes. An implementation of the mapping device which assumes its underlying buffer is continuous in memory will be provided.
