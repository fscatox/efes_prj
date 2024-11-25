/**
 * File              : math.svh
 * Author            : Fabio Scatozza <s315216@studenti.polito.it>
 * Date              : 22.11.2024
 * Description       : macros as a workaround for compatibility issues with real data types
 */

`ifndef MATH_SVH
`define MATH_SVH

// Returns longint'(x) as a real
`define REAL_INT64_TRUNC(x) real'(longint'(x))

// Returns smallest value not less than x
// longint ceil(real x)
`define CEIL(x) (\
  `REAL_INT64_TRUNC(x) == (x) ?\
    longint'(x)\
  :(\
    (x) > 0.0 ? (\
      longint'(x) + (`REAL_INT64_TRUNC(x) >= (x) ? 0 : 1)\
    ) : (\
      (x) == 0.0 ? 0 : (\
        longint'(x) + (`REAL_INT64_TRUNC(x) <= (x) ? 1 : 0)\
      )\
    )\
  )\
)

`endif // MATH_SVH

