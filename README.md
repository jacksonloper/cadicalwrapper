# cadicalwrapper

A WebAssembly (WASM) build of the [CaDiCaL](https://github.com/arminbiere/cadical) SAT solver, providing a minimal JavaScript interface for use in web applications.

## Interface

The module exposes a single function that accepts a SAT problem in DIMACS CNF format and returns variable assignments:

```js
const createCadical = require('./cadical.js');

const module = await createCadical();

const dimacs = `p cnf 3 2
1 -3 0
2 3 -1 0
`;

const resultPtr = module.ccall('solve_dimacs', 'number', ['string'], [dimacs]);
const resultStr = module.UTF8ToString(resultPtr);
module.ccall('free_result', null, ['number'], [resultPtr]);

const result = JSON.parse(resultStr);
// result = { satisfiable: true, values: [1, -2, 3] }
```

### Output format

The `solve_dimacs` function returns a JSON string:

- **SAT**: `{"satisfiable":true,"values":[1,-2,3]}` — each value is positive if the variable is true, negative if false.
- **UNSAT**: `{"satisfiable":false}`
- **Error**: `{"error":"description"}`

## Building

Requirements: [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)

```bash
make CXX=em++ all
```

Output files are placed in `build/`:
- `cadical.js` — JavaScript loader/glue
- `cadical.wasm` — WebAssembly binary

## CI / Releases

Every push to `main` builds the WASM artifacts. Pushing a tag like `v1.0.0` creates a GitHub Release with the built `cadical.js` and `cadical.wasm` attached, ready to be consumed from Netlify or any other deployment.