/*
*  File:        ptree-private.h
*  Description: Private functions for the PTree class
*  Date:        2022-03-06 03:30
*
*               DECLARE YOUR PTREE PRIVATE MEMBER FUNCTIONS HERE
*/

#ifndef _PTREE_PRIVATE_H_
#define _PTREE_PRIVATE_H_

/////////////////////////////////////////////////
// DEFINE YOUR PRIVATE MEMBER FUNCTIONS HERE
//
// Just write the function signatures.
//
// Example:
//
// Node* MyHelperFunction(int arg_a, bool arg_b);
//
/////////////////////////////////////////////////

void freeNode(Node* node);

Node* CopyNode(Node* node);

int CountNodes(Node* node) const;

int CountLeaves(Node* node) const;

void PruneSub(Node* node, double tol);

bool checkTolLeaves(Node * curr, double tol, HSLAPixel& avg);

void RenderNodes(Node* node, PNG& outpng) const;

void FlipXHelper(Node* subRoot);

void FlipYHelper(Node* subRoot);




#endif
