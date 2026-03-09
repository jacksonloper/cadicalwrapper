#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

#include "cadical/src/cadical.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT
#endif

extern "C" {

// Solve a SAT problem given in DIMACS CNF format.
// Returns a JSON string:
//   {"satisfiable":true,"values":[1,-2,3,...]}   if SAT
//   {"satisfiable":false}                        if UNSAT
//   {"error":"..."}                              on parse error
//
// The caller must free the returned string with free_result().
EXPORT
const char *solve_dimacs(const char *dimacs_input) {
  CaDiCaL::Solver solver;
  solver.set("quiet", 1);

  std::istringstream in(dimacs_input);
  std::string line;
  int num_vars = 0;
  int num_clauses = 0;

  while (std::getline(in, line)) {
    // skip empty lines and comments
    if (line.empty() || line[0] == 'c')
      continue;

    if (line[0] == 'p') {
      // parse header: "p cnf <vars> <clauses>"
      if (sscanf(line.c_str(), "p cnf %d %d", &num_vars, &num_clauses) < 2) {
        const char *err = "{\"error\":\"invalid problem line\"}";
        char *result = (char *)malloc(strlen(err) + 1);
        strcpy(result, err);
        return result;
      }
      // pre-declare all variables so CaDiCaL v3 doesn't complain
      if (num_vars > 0)
        solver.declare_more_variables(num_vars);
      continue;
    }

    // parse clause literals
    std::istringstream ls(line);
    int lit;
    while (ls >> lit) {
      solver.add(lit);
    }
  }

  int status = solver.solve();

  std::ostringstream out;
  if (status == 10) { // SATISFIABLE
    out << "{\"satisfiable\":true,\"values\":[";
    for (int i = 1; i <= num_vars; i++) {
      if (i > 1)
        out << ",";
      out << solver.val(i);
    }
    out << "]}";
  } else if (status == 20) { // UNSATISFIABLE
    out << "{\"satisfiable\":false}";
  } else {
    out << "{\"error\":\"solver returned unknown status\"}";
  }

  std::string s = out.str();
  char *result = (char *)malloc(s.size() + 1);
  strcpy(result, s.c_str());
  return result;
}

// Free a result string returned by solve_dimacs.
EXPORT
void free_result(const char *ptr) { free((void *)ptr); }
}
