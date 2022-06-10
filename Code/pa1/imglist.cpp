// File:        imglist.cpp
// Date:        2022-01-27 10:21
// Description: Contains partial implementation of ImgList class
//              for CPSC 221 2021W2 PA1
//              Function bodies to be completed by yourselves
//
// ADD YOUR FUNCTION IMPLEMENTATIONS IN THIS FILE
//

#include "imglist.h"

#include <math.h> // provides fmax, fmin, and fabs functions

/**************************
* MISCELLANEOUS FUNCTIONS *
**************************/

/*
* This function is NOT part of the ImgList class,
* but will be useful for one of the ImgList functions.
* Returns the "difference" between two hue values.
* PRE: hue1 is a double between [0,360).
* PRE: hue2 is a double between [0,360).
* 
* The hue difference is the absolute difference between two hues,
* but takes into account differences spanning the 360 value.
* e.g. Two pixels with hues 90 and 110 differ by 20, but
*      two pixels with hues 5 and 355 differ by 10.
*/
double HueDiff(double hue1, double hue2) {
  return fmin(fabs(hue1 - hue2), fabs(360 + fmin(hue1, hue2) - fmax(hue1, hue2)));
}

/*********************
* CONSTRUCTORS, ETC. *
*********************/

/*
* Default constructor. Makes an empty list
*/
ImgList::ImgList() {
  // set appropriate values for all member attributes here
  northwest = NULL; // entry point to the list; the upper-left corner of the image
  southeast = NULL; // last node in the list; the lower-right corner of the image
  
}

/*
* Creates a list from image data
* PRE: img has dimensions of at least 1x1
*/
ImgList::ImgList(PNG& img) {
  // build the linked node structure and set the member attributes appropriately
  ImgNode* prev;
  ImgNode* curr;
  ImgNode* currRow;
  ImgNode* prevRow;

  for (unsigned y = 0; y < img.height(); y++) {
    for (unsigned x = 0; x < img.width(); x++) {
      curr = new ImgNode();
      curr->colour = *img.getPixel(x, y);
      if (x == 0 && y == 0) {
        northwest = curr;
      }
      if (x == 0) {
        currRow = curr;
        prev = curr;
      }
      if (y > 0) {
        curr->north = prevRow;
        prevRow->south = curr;
        prevRow = prevRow->east;
      }
      if (x > 0) {
        curr->west = prev;
        prev->east = curr;
        prev = curr;
      }
      if (x == img.width() - 1) {
        prevRow = currRow;
      }
    }
  }
  southeast = curr;
}

/*
* Copy constructor.
* Creates this this to become a separate copy of the data in otherlist
*/
ImgList::ImgList(const ImgList& otherlist) {
  // build the linked node structure using otherlist as a template
  Copy(otherlist);
}

/*
* Assignment operator. Enables statements such as list1 = list2;
*   where list1 and list2 are both variables of ImgList type.
* POST: the contents of this list will be a physically separate copy of rhs
*/
ImgList& ImgList::operator=(const ImgList& rhs) {
  // Re-build any existing structure using rhs as a template
  
  if (this != &rhs) { // if this list and rhs are different lists in memory
    // release all existing heap memory of this list
    Clear();    
    
    // and then rebuild this list using rhs as a template
    Copy(rhs);
  }
  
  return *this;
}

/*
* Destructor.
* Releases any heap memory associated with this list.
*/
ImgList::~ImgList() {
  // Ensure that any existing heap memory is deallocated
  Clear();
}

/************
* ACCESSORS *
************/

/*
* Returns the horizontal dimension of this list (counted in nodes)
* Note that every row will contain the same number of nodes, whether or not
*   the list has been carved.
* We expect your solution to take linear time in the number of nodes in the
*   x dimension.
*/
unsigned int ImgList::GetDimensionX() const {
  // replace the following line with your implementation
  if (northwest == NULL) {
    return 0;
  }
  int i = 1;
  ImgNode* temp = northwest;
  while (temp->east != NULL) {
    i++;
    temp = temp->east;
  }
  temp = NULL;
  return i;
}

/*
* Returns the vertical dimension of the list (counted in nodes)
* It is useful to know/assume that the grid will never have nodes removed
*   from the first or last columns. The returned value will thus correspond
*   to the height of the PNG image from which this list was constructed.
* We expect your solution to take linear time in the number of nodes in the
*   y dimension.
*/
unsigned int ImgList::GetDimensionY() const {
  // replace the following line with your implementation
  if (northwest == NULL) {
    return 0;
  }
  int i = 1;
  ImgNode* temp = northwest;
  while (temp->south != NULL) {
    i++;
    temp = temp->south;
  }
  temp = NULL;
  return i;
}
/*
* Returns the horizontal dimension of the list (counted in original pixels, pre-carving)
* The returned value will thus correspond to the width of the PNG image from
*   which this list was constructed.
* We expect your solution to take linear time in the number of nodes in the
*   x dimension.
*/
unsigned int ImgList::GetDimensionFullX() const {
  // replace the following line with your implementation
  if (northwest == NULL) {
    return 0;
  }
  int result = 0;
  ImgNode* curr = northwest;

  while (curr->east != NULL) {
    result += curr->skipright;
    result++;
    curr = curr->east;
  }

  return result + 1;
}

/*
* Returns a pointer to the node which best satisfies the specified selection criteria.
* The first and last nodes in the row cannot be returned.
* PRE: rowstart points to a row with at least 3 physical nodes
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: rowstart - pointer to the first node in a row
* PARAM: selectionmode - criterion used for choosing the node to return
*          0: minimum luminance across row, not including extreme left or right nodes
*          1: node with minimum total of "hue difference" with its left neighbour and with its right neighbour.
*        In the (likely) case of multiple candidates that best match the criterion,
*        the left-most node satisfying the criterion (excluding the row's starting node)
*        will be returned.
* A note about "hue difference": For PA1, consider the hue value to be a double in the range [0, 360).
* That is, a hue value of exactly 360 should be converted to 0.
* The hue difference is the absolute difference between two hues,
* but be careful about differences spanning the 360 value.
* e.g. Two pixels with hues 90 and 110 differ by 20, but
*      two pixels with hues 5 and 355 differ by 10.
*/
ImgNode* ImgList::SelectNode(ImgNode* rowstart, int selectionmode) {
  // add your implementation below
  ImgNode* curr = rowstart->east;
  ImgNode* min = curr;   
  if (selectionmode == 0) {
    while (curr->east != NULL) {
      if (curr->colour.l < min->colour.l) {
        min = curr;
      }
      curr = curr->east;
    }
  } else {
    while (curr->east != NULL) {
      // double diff1 = fabs(curr->west->colour.h - curr->east->colour.h);
      // diff1 = fmin(diff1, fabs(360 - diff1));
      // double diff2 = fabs(min->west->colour.h - min->east->colour.h);
      // diff2 = fmin(diff2, fabs(360 - diff2));
      double diff1 = HueDiff(curr->west->colour.h, curr->colour.h);
      diff1 += HueDiff(curr->east->colour.h, curr->colour.h);

      double diff2 = HueDiff(min->west->colour.h, min->colour.h);
      diff2 += HueDiff(min->east->colour.h, min->colour.h);
      
      if (fabs(diff1 - diff2) > 0.000001 && diff1 < diff2 - 0.000001) {
        min = curr;
      }
      curr = curr->east;
    }
  }
  return min;
}

/*
* Renders this list's pixel data to a PNG, with or without filling gaps caused by carving.
* PRE: fillmode is an integer in the range of [0,2]
* PARAM: fillgaps - whether or not to fill gaps caused by carving
*          false: render one pixel per node, ignores fillmode
*          true: render the full width of the original image,
*                filling in missing nodes using fillmode
* PARAM: fillmode - specifies how to fill gaps
*          0: solid, uses the same colour as the node at the left of the gap
*          1: solid, using the averaged values (all channels) of the nodes at the left and right of the gap
*             Note that "average" for hue will use the closer of the angular distances,
*             e.g. average of 10 and 350 will be 0, instead of 180.
*             Average of diametric hue values will use the smaller of the two averages
*             e.g. average of 30 and 210 will be 120, and not 300
*                  average of 170 and 350 will be 80, and not 260
*          2: *** OPTIONAL - FOR BONUS ***
*             linear gradient between the colour (all channels) of the nodes at the left and right of the gap
*             e.g. a gap of width 1 will be coloured with 1/2 of the difference between the left and right nodes
*             a gap of width 2 will be coloured with 1/3 and 2/3 of the difference
*             a gap of width 3 will be coloured with 1/4, 2/4, 3/4 of the difference, etc.
*             Like fillmode 1, use the smaller difference interval for hue,
*             and the smaller-valued average for diametric hues
*/
PNG ImgList::Render(bool fillgaps, int fillmode) const {
  // Add/complete your implementation below
  PNG outpng;
  ImgNode* curr = northwest;
  ImgNode* currRowFirst;
  if (!fillgaps) {
    outpng = PNG(GetDimensionX(), GetDimensionY());
    for (unsigned y = 0; y < outpng.height(); y++) {
      for (unsigned x = 0; x < outpng.width(); x++) {
        HSLAPixel *pixel = outpng.getPixel(x, y);
        pixel->h = curr->colour.h;
        pixel->s = curr->colour.s;
        pixel->l = curr->colour.l;
        pixel->a = curr->colour.a;
        if (x == 0) {
          currRowFirst = curr;
        }
        if (x < outpng.width() - 1) {
          curr = curr->east;
        } else if (x == outpng.width() - 1 && y < outpng.height() - 1) {
          curr = currRowFirst->south;
        }
      }
    }
  } else if (fillgaps) {
    outpng = PNG(GetDimensionFullX(), GetDimensionY());
    for (unsigned y = 0; y < outpng.height(); y++) {
      for (unsigned x = 0; x < outpng.width(); x++) {
        HSLAPixel *pixel = outpng.getPixel(x, y);
        pixel->h = curr->colour.h;
        pixel->s = curr->colour.s;
        pixel->l = curr->colour.l;
        pixel->a = curr->colour.a;
        if (x == 0) {
          currRowFirst = curr;
        }
        if (curr->skipright != 0) {
          x++;
          for (int i = x; i < x + curr->skipright; i++) {
            HSLAPixel *pixeln = outpng.getPixel(i, y);
            if (fillmode == 0) {
              pixeln->h = curr->colour.h;
              pixeln->s = curr->colour.s;
              pixeln->l = curr->colour.l;
              pixeln->a = curr->colour.a;
            } else if (fillmode == 1) {
              double avgh;
              double diff = fabs(curr->colour.h - curr->east->colour.h);
              if (diff > 180 - 0.000001) {
                avgh = ((fmax(curr->colour.h, curr->east->colour.h) - 360) + fmin(curr->colour.h, curr->east->colour.h));
                avgh = avgh / 2.0;
                if (avgh < 0) {
                  avgh = 360 + avgh;
                }
              } else if (diff > 180 - 0.000001 && diff < 180 + 0.00001) {
                avgh = ((fmax(curr->colour.h, curr->east->colour.h) - 360) + fmin(curr->colour.h, curr->east->colour.h));
                avgh = avgh / 2.0;
                if (avgh < 0) {
                  avgh = 360 + avgh;
                }
                double avg2 = (curr->colour.h + curr->east->colour.h) / 2.0;
                avgh = fmin(avgh, avg2);
              } else {
                avgh = (curr->colour.h + curr->east->colour.h) / 2.0;
              }
              pixeln->h = avgh;

              double avgs = (curr->colour.s + curr->east->colour.s) / 2.0;
              double avgl = (curr->colour.l + curr->east->colour.l) / 2.0;
              double avga = (curr->colour.a + curr->east->colour.a) / 2.0;
              pixeln->s = avgs;
              pixeln->l = avgl;
              pixeln->a = avga;
            }
            
          }
          x += curr->skipright - 1;
        }
        
        if (x < outpng.width() - 1) {
          curr = curr->east;
        } else if (x == outpng.width() - 1 && y < outpng.height() - 1) {
          curr = currRowFirst->south;
        }
      }
    }
  }
  
   //this will be returned later. Might be a good idea to resize it at some point.
  
  return outpng;
}

/************
* MODIFIERS *
************/

/*
* Removes exactly one node from each row in this list, according to specified criteria.
* The first and last nodes in any row cannot be carved.
* PRE: this list has at least 3 nodes in each row
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: selectionmode - see the documentation for the SelectNode function.
* POST: this list has had one node removed from each row. Neighbours of the created
*       gaps are linked appropriately, and their skip values are updated to reflect
*       the size of the gap.
*/
void ImgList::Carve(int selectionmode) {
  // add your implementation here
  ImgNode* curr = northwest;
  for (int i = 0; i < GetDimensionY(); i++) {
    ImgNode* toDelete = SelectNode(curr, selectionmode);
    if (toDelete->north != NULL && toDelete->south != NULL) {
      toDelete->north->skipdown += toDelete->skipdown + 1;
      toDelete->south->skipup += toDelete->skipup + 1;
      toDelete->north->south = toDelete->south;
      toDelete->south->north = toDelete->north;
    } else if (toDelete->north == NULL && toDelete->south != NULL) {
      toDelete->south->skipup += toDelete->skipup + 1;
      toDelete->south->north = NULL;
    } else if (toDelete->south == NULL && toDelete->north != NULL) {
      toDelete->north->skipdown += toDelete->skipdown + 1;
      toDelete->north->south = NULL;
    }
    // if (toDelete->east != NULL && toDelete->west != NULL) {
    toDelete->east->skipleft += toDelete->skipleft + 1;
    toDelete->west->skipright += toDelete->skipright + 1;
    toDelete->east->west = toDelete->west;
    toDelete->west->east = toDelete->east;
    // }
    delete toDelete;
    toDelete = NULL;
    // if (curr->south != NULL) {
    curr = curr->south;
    // }
  }
  curr = NULL;
}

// note that a node on the boundary will never be selected for removal
/*
* Removes "rounds" number of nodes (up to a maximum of node width - 2) from each row,
* based on specific selection criteria.
* Note that this should remove one node from every row, repeated "rounds" times,
* and NOT remove "rounds" nodes from one row before processing the next row.
* PRE: selectionmode is an integer in the range [0,1]
* PARAM: rounds - number of nodes to remove from each row
*        If rounds exceeds node width - 2, then remove only node width - 2 nodes from each row.
*        i.e. Ensure that the final list has at least two nodes in each row.
* POST: this list has had "rounds" nodes removed from each row. Neighbours of the created
*       gaps are linked appropriately, and their skip values are updated to reflect
*       the size of the gap.
*/
void ImgList::Carve(unsigned int rounds, int selectionmode) {
  // add your implementation here
  if (rounds > GetDimensionFullX() - 2) {
      for (int i = 0; i < GetDimensionFullX() - 2; i++) {
        Carve(selectionmode);
    } 
  } else {
    for (int i = 0; i < rounds; i++) {
        Carve(selectionmode);
    }
  }
}


/*
* Helper function deallocates all heap memory associated with this list,
* puts this list into an "empty" state. Don't forget to set your member attributes!
* POST: this list has no currently allocated nor leaking heap memory,
*       member attributes have values consistent with an empty list.
*/
void ImgList::Clear() {
  // add your implementation here
  ImgNode* curr = northwest;
  ImgNode* nextRow;
  while (curr != southeast) {
    if (curr->south != NULL) {
      nextRow = curr->south;
    } 
    while (curr->east != NULL) {
      ImgNode* temp = curr->east;
      delete curr;
      curr = temp;
    }
    if (curr == southeast) {
      break;
    }
    delete curr;
    curr = nextRow;
  }
  delete curr;
  southeast = NULL;    
  northwest = NULL;
  curr = NULL;
  nextRow = NULL;


  // while (curr != NULL) {

  //   if (curr->south != NULL) {
  //     nextRow = curr->south;
  //   } else {
  //     nextRow = NULL;
  //   }
  //   while (curr != NULL) {
  //     if (curr->east == NULL) {
  //       delete curr;
  //       curr = NULL;
  //       break;
  //     }
  //     ImgNode* temp = curr->east;
  //     delete curr;
  //     curr = temp;
  //   }
  //   curr = nextRow;
  // }
  // northwest = NULL;
  // southeast = NULL;

  
}

/* ************************
*  * OPTIONAL - FOR BONUS *
** ************************
* Helper function copies the contents of otherlist and sets this list's attributes appropriately
* PRE: this list is empty
* PARAM: otherlist - list whose contents will be copied
* POST: this list has contents copied from by physically separate from otherlist
*/
void ImgList::Copy(const ImgList& otherlist) {
  // add your implementation here
  // ImgNode& ref = *otherlist.northwest;
  // ImgNode* curr;
  // while (ref.east != NULL) {
  //   curr = new ImgNode(ref);
  //   curr = ref.east;
  

  // }


  // ImgNode* prev;
  // ImgNode* curr;
  // ImgNode* firstCurrRow;
  // ImgNode* prevRow;
  // ImgNode* oprev;
  // ImgNode* ocurr = otherlist.northwest;
  // ImgNode* ocurrRow;
  // ImgNode* oprevRow;

  // for (unsigned y = 0; y < otherlist.GetDimensionY(); y++) {
  //   for (unsigned x = 0; x < otherlist.GetDimensionFullX(); x++) {
  //     curr = new ImgNode();
  //     curr->colour = ocurr->colour;
  //     if (x == 0 && y == 0) {
  //       northwest = curr;
  //     }
  //     if (x == 0) {
  //       firstCurrRow = curr;
  //       ocurrRow = ocurr;
  //       prev = curr;
  //     }
  //     if (y > 0) {
  //       curr->north = ocurr->north;
  //       prevRow->south = oprevRow->south;
  //       prevRow = prevRow->east;
  //       oprevRow = oprevRow->east;
  //     }
  //     if (x > 0) {
  //       curr->west = prev;
  //       prev->east = curr;
  //       prev = curr;
  //       oprev = ocurr;
  //     }
  //     if (x == otherlist.GetDimensionY() - 1) {
  //       prevRow = firstCurrRow;
  //       oprevRow = ocurrRow;
  //     }
  //   }
  // }
  // southeast = curr;
  
}

/*************************************************************************************************
* IF YOU DEFINED YOUR OWN PRIVATE FUNCTIONS IN imglist.h, YOU MAY ADD YOUR IMPLEMENTATIONS BELOW *
*************************************************************************************************/

