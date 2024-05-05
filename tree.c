#include "defs.h"
#include "data.h"
#include "decl.h"

// AST tree functions
// Copyright (c) 2019 Warren Toomey, GPL3

// Build and return a generic AST node
ast *mkastnode(int op, ast *left,
			  ast *right, int intvalue) {
  ast *n;

  // Malloc a new ASTnode
  n = (ast *) malloc(sizeof(ast));
  if (n == NULL)
    fatal("Unable to malloc in mkastnode()");

  // Copy in the field values and return it
  n->op = op;
  n->left = left;
  n->right = right;
  n->v.intvalue = intvalue;
  return (n);
}


// Make an AST leaf node
ast *mkastleaf(int op, int intvalue) {
  return (mkastnode(op, NULL, NULL, intvalue));
}

// Make a unary AST node: only one child
ast *mkastunary(int op, ast *left, int intvalue) {
  return (mkastnode(op, left, NULL, intvalue));
}
