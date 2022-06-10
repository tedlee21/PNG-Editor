/*
*  File:        ptree.cpp
*  Description: Implementation of a partitioning tree class for CPSC 221 PA3
*  Date:        2022-03-03 01:53
*
*               ADD YOUR PRIVATE FUNCTION IMPLEMENTATIONS TO THE BOTTOM OF THIS FILE
*/

#include <stdlib.h>     /* malloc, free, rand */
#include "ptree.h"
#include "hue_utils.h" // useful functions for calculating hue averages

using namespace cs221util;
using namespace std;

// The following definition may be convenient, but is not necessary to use
typedef pair<unsigned int, unsigned int> pairUI;

/////////////////////////////////
// PTree private member functions
/////////////////////////////////

/*
*  Destroys all dynamically allocated memory associated with the current PTree object.
*  You may want to add a recursive helper function for this!
*  POST: all nodes allocated into the heap have been released.
*/
void PTree::Clear() {
  // add your implementation below
  freeNode(root);
  root = nullptr;
}

void PTree::freeNode(Node* node) {
  // add your implementation below
  if (node == nullptr) {
    return;
  }
    freeNode(node->A);
    freeNode(node->B);

    delete(node);
    node = nullptr;
}

/*
*  Copies the parameter other PTree into the current PTree.
*  Does not free any memory. Should be called by copy constructor and operator=.
*  You may want a recursive helper function for this!
*  PARAM: other - the PTree which will be copied
*  PRE:   There is no dynamic memory associated with this PTree.
*  POST:  This PTree is a physically separate copy of the other PTree.
*/
void PTree::Copy(const PTree& other) {
  // add your implementation below
  root = CopyNode(other.root);
}

Node* PTree::CopyNode(Node* node) {
  if (node == nullptr) {
    return nullptr;
  }
  
  Node* newNode = new Node(node->upperleft, node->width, node->height, node->avg);
  newNode->A = CopyNode(node->A);
  newNode->B = CopyNode(node->B);

  return newNode;
}

/*
*  Private helper function for the constructor. Recursively builds the tree
*  according to the specification of the constructor.
*  You *may* change this if you like, but we have provided here what we
*  believe will be sufficient to use as-is.
*  PARAM:  im - full reference image used for construction
*  PARAM:  ul - upper-left image coordinate of the currently building Node's image region
*  PARAM:  w - width of the currently building Node's image region
*  PARAM:  h - height of the currently building Node's image region
*  RETURN: pointer to the fully constructed Node
*/
Node* PTree::BuildNode(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {
  // replace the line below with your implementation
  HSLAPixel avg;

  if (w == 1 && h == 1) {
    avg = *im.getPixel(ul.first, ul.second);
    
    //Node* ret = (Node*) malloc(sizeof(*ret));
    Node* ret = new Node(ul, w, h, avg, nullptr, nullptr);
    // cout<< "got to ";
    // cout<< ul.first;
    // cout<< ul.second;
    // cout<<"\n";
    
    return ret;
  }
  
  bool horizontal = true;
  bool equaldiv = false;
  unsigned int retwa = 0;
  unsigned int retwb = 0;

  unsigned int retha = 0;
  unsigned int rethb = 0;

  double avghx = 0;
  double avghy = 0;
  double avgs = 0;
  double avgl = 0;
  double avga = 0;

  for (unsigned x = ul.first; x < ul.first + w; x++) {
    for (unsigned y = ul.second; y < ul.second + h; y++) {
  
      HSLAPixel pixel = *im.getPixel(x, y);
      avghx += Deg2X(pixel.h);
      avghy += Deg2Y(pixel.h);
      avgs += pixel.s;
      avgl += pixel.l;
      avga += pixel.a;

    }
  }
  avghx = avghx/ (w * h);
  avghy = avghy/ (w * h);
  avgs = avgs/ (w * h);
  avgl = avgl/ (w * h);
  avga = avga/ (w * h);
  avg.h = XY2Deg(avghx, avghy);
  avg.s = avgs;
  avg.l = avgl;
  avg.a = avga;

  // determines whether division will be horizontal or not, and equal or not
  // if (h == w) then horizontal stays true
  if (h > w) {
    horizontal = false;
    equaldiv = (h%2 == 0);
  } else {
    equaldiv = (w%2 == 0);
  }

  // determines return widths and heights of A and B regions
  if (horizontal && equaldiv) {
    retwa = w/2;
    retwb = w/2;

    retha = h;
    rethb = h;
  } else if (horizontal && !equaldiv) {
    retwa = (w-1)/2;
    retwb = (w+1)/2;

    retha = h;
    rethb = h;
  } else if (!horizontal && equaldiv) {
    retha = h/2;
    rethb = h/2;

    retwa = w;
    retwb = w;
  } else {
    retha = (h-1)/2;
    rethb = (h+1)/2;

    retwa = w;
    retwb = w;
  }

  pair<unsigned int, unsigned int> bUl = make_pair(0,0);
  if (horizontal) {
    bUl = make_pair((unsigned int) ul.first + retwa, (unsigned int) ul.second);
  } else {
    bUl = make_pair((unsigned int) ul.first, (unsigned int) ul.second + retha);
  }
  
  //LEAK

  Node* A = BuildNode(im, ul, retwa, retha);
  // cout<<"Finished A\n";

  Node* B = BuildNode(im, bUl, retwb, rethb);
  // cout<<"Finished B\n";

  Node* ret = new Node(ul, w, h, avg, A, B);
  // cout<< "returning parent\n";
  return ret;
}

////////////////////////////////
// PTree public member functions
////////////////////////////////

/*
*  Constructor that builds the PTree using the provided PNG.
*
*  The PTree represents the sub-image (actually the entire image) from (0,0) to (w-1, h-1) where
*  w-1 and h-1 are the largest valid image coordinates of the original PNG.
*  Each node corresponds to a rectangle of pixels in the original PNG, represented by
*  an (x,y) pair for the upper-left corner of the rectangle, and two unsigned integers for the
*  number of pixels on the width and height dimensions of the rectangular sub-image region the
*  node defines.
*
*  A node's two children correspond to a partition of the node's rectangular region into two
*  equal (or approximately equal) size regions which are either tiled horizontally or vertically.
*
*  If the rectangular region of a node is taller than it is wide, then its two children will divide
*  the region into vertically-tiled sub-regions of equal height:
*  +-------+
*  |   A   |
*  |       |
*  +-------+
*  |   B   |
*  |       |
*  +-------+
*
*  If the rectangular region of a node is wider than it is tall, OR if the region is exactly square,
*  then its two children will divide the region into horizontally-tiled sub-regions of equal width:
*  +-------+-------+
*  |   A   |   B   |
*  |       |       |
*  +-------+-------+
*
*  If any region cannot be divided exactly evenly (e.g. a horizontal division of odd width), then
*  child B will receive the larger half of the two subregions.
*
*  When the tree is fully constructed, each leaf corresponds to a single pixel in the PNG image.
*
*  For the average colour, this MUST be computed separately over the node's rectangular region.
*  Do NOT simply compute this as a weighted average of the children's averages.
*  The functions defined in hue_utils.h and implemented in hue_utils.cpp will be very useful.
*  Computing the average over many overlapping rectangular regions sounds like it will be
*  inefficient, but as an exercise in theory, think about the asymptotic upper bound on the
*  number of times any given pixel is included in an average calculation.
*
*  PARAM: im - reference image which will provide pixel data for the constructed tree's leaves
*  POST:  The newly constructed tree contains the PNG's pixel data in each leaf node.
*/
PTree::PTree(PNG& im) {
  // add your implementation below
  pair<unsigned int, unsigned int> ul = make_pair((unsigned int) 0, (unsigned int) 0);
  root = BuildNode(im, ul, im.width(), im.height());
}

/*
*  Copy constructor
*  Builds a new tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  This tree is constructed as a physically separate copy of other tree.
*/
PTree::PTree(const PTree& other) {
  // add your implementation below
  Copy(other);
}

/*
*  Assignment operator
*  Rebuilds this tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  If other is a physically different tree in memory, all pre-existing dynamic
*           memory in this tree is deallocated and this tree is reconstructed as a
*           physically separate copy of other tree.
*         Otherwise, there is no change to this tree.
*/
PTree& PTree::operator=(const PTree& other) {
  // add your implementation below
  if (this == &other) {
    return *this;
  }
  Clear();
  Copy(other);
  return *this;
}

/*
*  Destructor
*  Deallocates all dynamic memory associated with the tree and destroys this PTree object.
*/
PTree::~PTree() {
  // add your implementation below
  Clear();

}

/*
*  Traverses the tree and puts the leaf nodes' color data into the nodes'
*  defined image regions on the output PNG.
*  For non-pruned trees, each leaf node corresponds to a single pixel that will be coloured.
*  For pruned trees, each leaf node may cover a larger rectangular region that will be
*  entirely coloured using the node's average colour attribute.
*
*  You may want to add a recursive helper function for this!
*
*  RETURN: A PNG image of appropriate dimensions and coloured using the tree's leaf node colour data
*/
PNG PTree::Render() const {
  // replace the line below with your implementation
  PNG outpng = PNG(root->width, root->height);
  RenderNodes(root, outpng);
  return outpng;
}

void PTree::RenderNodes(Node* node, PNG& outpng) const{
  if (node->A == nullptr) {
    for (unsigned int x = node->upperleft.first; x < node->upperleft.first + node->width; x++) {
      for (unsigned int y = node->upperleft.second; y < node->upperleft.second + node->height; y++) {
        HSLAPixel *pixel = outpng.getPixel(x, y);
        pixel->h = node->avg.h;
        pixel->s = node->avg.s;
        pixel->l = node->avg.l;
        pixel->a = node->avg.a;
      }
    }

  } else {
    RenderNodes(node->A, outpng);
    RenderNodes(node->B, outpng);
  }
}

/*
*  Trims subtrees as high as possible in the tree. A subtree is pruned
*  (its children are cleared/deallocated) if ALL of its leaves have colour
*  within tolerance of the subtree root's average colour.
*  Pruning criteria should be evaluated on the original tree, and never on a pruned
*  tree (i.e. we expect that Prune would be called on any tree at most once).
*  When processing a subtree, you should determine if the subtree should be pruned,
*  and prune it if possible before determining if it has subtrees that can be pruned.
*
*  You may want to add (a) recursive helper function(s) for this!
*
*  PRE:  This tree has not been previously pruned (and is not copied/assigned from a tree that has been pruned)
*  POST: Any subtrees (as close to the root as possible) whose leaves all have colour
*        within tolerance from the subtree's root colour will have their children deallocated;
*        Each pruned subtree's root becomes a leaf node.
*/
void PTree::Prune(double tolerance) {
  // add your implementation below
  PruneSub(root, tolerance);
}


bool PTree::checkTolLeaves(Node * curr, double tol, HSLAPixel& avg){
  if (curr->A == nullptr && curr->B == nullptr) {
    return (avg.dist(curr->avg) <= tol);
  } else {
    return (checkTolLeaves(curr->A, tol, avg) && checkTolLeaves(curr->B, tol, avg));
  }
}


void PTree::PruneSub(Node* node, double tol) {
  if (node == nullptr) {
    return;
  }

  if(checkTolLeaves(node, tol, node->avg)) {
    freeNode(node->A);
    node->A = nullptr;
    freeNode(node->B);
    node->B = nullptr;
  } else {
    PruneSub(node->A, tol);
    PruneSub(node->B, tol);
  }
}



/*
*  Returns the total number of nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::Size() const {
  // replace the line below with your implementation
  int sum = CountNodes(root);
  return sum;
}

int PTree::CountNodes(Node* node) const {
  if (node == nullptr) {
    return 0;
  }

  return 1 + CountNodes(node->A) + CountNodes(node->B);
}

/*
*  Returns the total number of leaf nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::NumLeaves() const {
  // replace the line below with your implementation
  int sum = CountLeaves(root);
  return sum;
}

int PTree::CountLeaves(Node* node) const {
  if (node->A == nullptr) {
    return 1;
  }
  int sum = CountLeaves(node->A);
  sum += CountLeaves(node->B);

  return sum;
}


/*
*  Rearranges the nodes in the tree, such that a rendered PNG will be flipped horizontally
*  (i.e. mirrored over a vertical axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped horizontally.
*/
void PTree::FlipHorizontal() {
  // add your implementation below
  // FlipXHelper(root);

  PNG img = Render();

  unsigned int sizeX = img.width();
  unsigned int sizeY = img.height();

  for (int x = 0; x < sizeX / 2; x++) {
    for (int y = 0; y < sizeY; y++ ) {

      HSLAPixel *pix = img.getPixel(x, y);
      HSLAPixel pixtemp = *(img.getPixel(x, y));

      HSLAPixel *pix2 = img.getPixel(sizeX - x - 1, y);

      *(img.getPixel(x, y)) = *pix2;
      *(img.getPixel(sizeX - x - 1, y)) = pixtemp;
    }
  }
  Clear();
  root = BuildNode(img, make_pair((unsigned int) 0, (unsigned int) 0), sizeX, sizeY);
}

void PTree::FlipXHelper(Node* subRoot) {
  if (subRoot->A == nullptr && subRoot->B == nullptr) {
    int mid;
    if (root->width % 2 == 0) {
      mid = root->width / 2;
      subRoot->upperleft = make_pair((mid - subRoot->upperleft.first) + mid - 1, subRoot->upperleft.second);
      
    } else {
      mid = (root->width - 1) / 2;
      if (subRoot->upperleft.first != mid) {
        subRoot->upperleft = make_pair((mid - subRoot->upperleft.first) + mid, subRoot->upperleft.second);
      }
    }

  } else {
    FlipXHelper(subRoot->A);
    FlipXHelper(subRoot->B);
  }

}

/*
*  Like the function above, rearranges the nodes in the tree, such that a rendered PNG
*  will be flipped vertically (i.e. mirrored over a horizontal axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped vertically.
*/
void PTree::FlipVertical() {
  // add your implementation below
  // FlipYHelper(root);
  PNG img = Render();

  unsigned int sizeX = img.width();
  unsigned int sizeY = img.height();

  for (int x = 0; x < sizeX; x++) {
    for (int y = 0; y < sizeY / 2; y++) {

      HSLAPixel *pix = img.getPixel(x, y);
      HSLAPixel pixtemp = *(img.getPixel(x, y));

      HSLAPixel *pix2 = img.getPixel(x, sizeY - y -1);

      *(img.getPixel(x, y)) = *pix2;
      *(img.getPixel(x, sizeY - y - 1)) = pixtemp;
    }
  }
  Clear();
  root = BuildNode(img, make_pair((unsigned int) 0, (unsigned int) 0), sizeX, sizeY);

}

void PTree::FlipYHelper(Node* subRoot) {
  if (subRoot->A == nullptr && subRoot->B == nullptr) {
    int mid;
    if (root->height % 2 == 0) {
      mid = root->height / 2;
      subRoot->upperleft = make_pair(subRoot->upperleft.first, (mid - subRoot->upperleft.second) + mid - 1);
      
    } else {
      mid = (root->height - 1) / 2;
      if (subRoot->upperleft.second != mid) {
        subRoot->upperleft = make_pair(subRoot->upperleft.first, (mid - subRoot->upperleft.second) + mid);
      }
    }

  } else {
    FlipYHelper(subRoot->A);
    FlipYHelper(subRoot->B);
  }

}

/*
    *  Provides access to the root of the tree.
    *  Dangerous in practice! This is only used for testing.
    */
Node* PTree::GetRoot() {
  return root;
}

//////////////////////////////////////////////
// PERSONALLY DEFINED PRIVATE MEMBER FUNCTIONS
//////////////////////////////////////////////

