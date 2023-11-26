/// image8bit - A simple image processing module.
///
/// This module is part of a programming project
/// for the course AED, DETI / UA.PT
///
/// You may freely use and modify this code, at your own risk,
/// as long as you give proper credit to the original and subsequent authors.
///
/// João Manuel Rodrigues <jmr@ua.pt>
/// 2013, 2023

// Student authors (fill in below):
// NMec: 102659
// 
// Name: Carlos Moura
// 
// Date:
//

#include "image8bit.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"
#include <math.h>

// The data structure
//
// An image is stored in a structure containing 3 fields:
// Two integers store the image width and height.
// The other field is a pointer to an array that stores the 8-bit gray
// level of each pixel in the image.  The pixel array is one-dimensional
// and corresponds to a "raster scan" of the image from left to right,
// top to bottom.
// For example, in a 100-pixel wide image (img->width == 100),
//   pixel position (x,y) = (33,0) is stored in img->pixel[33];
//   pixel position (x,y) = (22,1) is stored in img->pixel[122].
// 
// Clients should use images only through variables of type Image,
// which are pointers to the image structure, and should not access the
// structure fields directly.

// Maximum value you can store in a pixel (maximum maxval accepted)
const uint8 PixMax = 255;

// Internal structure for storing 8-bit graymap images
struct image {
  int width;
  int height;
  int maxval;   // maximum gray value (pixels with maxval are pure WHITE)
  uint8* pixel; // pixel data (a raster scan)
};


// This module follows "design-by-contract" principles.
// Read `Design-by-Contract.md` for more details.

/// Error handling functions

// In this module, only functions dealing with memory allocation or file
// (I/O) operations use defensive techniques.
// 
// When one of these functions fails, it signals this by returning an error
// value such as NULL or 0 (see function documentation), and sets an internal
// variable (errCause) to a string indicating the failure cause.
// The errno global variable thoroughly used in the standard library is
// carefully preserved and propagated, and clients can use it together with
// the ImageErrMsg() function to produce informative error messages.
// The use of the GNU standard library error() function is recommended for
// this purpose.
//
// Additional information:  man 3 errno;  man 3 error;

// Variable to preserve errno temporarily
static int errsave = 0;

// Error cause
static char* errCause;

/// Error cause.
/// After some other module function fails (and returns an error code),
/// calling this function retrieves an appropriate message describing the
/// failure cause.  This may be used together with global variable errno
/// to produce informative error messages (using error(), for instance).
///
/// After a successful operation, the result is not garanteed (it might be
/// the previous error cause).  It is not meant to be used in that situation!
char* ImageErrMsg() { ///
  return errCause;
}


// Defensive programming aids
//
// Proper defensive programming in C, which lacks an exception mechanism,
// generally leads to possibly long chains of function calls, error checking,
// cleanup code, and return statements:
//   if ( funA(x) == errorA ) { return errorX; }
//   if ( funB(x) == errorB ) { cleanupForA(); return errorY; }
//   if ( funC(x) == errorC ) { cleanupForB(); cleanupForA(); return errorZ; }
//
// Understanding such chains is difficult, and writing them is boring, messy
// and error-prone.  Programmers tend to overlook the intricate details,
// and end up producing unsafe and sometimes incorrect programs.
//
// In this module, we try to deal with these chains using a somewhat
// unorthodox technique.  It resorts to a very simple internal function
// (check) that is used to wrap the function calls and error tests, and chain
// them into a long Boolean expression that reflects the success of the entire
// operation:
//   success = 
//   check( funA(x) != error , "MsgFailA" ) &&
//   check( funB(x) != error , "MsgFailB" ) &&
//   check( funC(x) != error , "MsgFailC" ) ;
//   if (!success) {
//     conditionalCleanupCode();
//   }
//   return success;
// 
// When a function fails, the chain is interrupted, thanks to the
// short-circuit && operator, and execution jumps to the cleanup code.
// Meanwhile, check() set errCause to an appropriate message.
// 
// This technique has some legibility issues and is not always applicable,
// but it is quite concise, and concentrates cleanup code in a single place.
// 
// See example utilization in ImageLoad and ImageSave.
//
// (You are not required to use this in your code!)


// Check a condition and set errCause to failmsg in case of failure.
// This may be used to chain a sequence of operations and verify its success.
// Propagates the condition.
// Preserves global errno!
static int check(int condition, const char* failmsg) {
  errCause = (char*)(condition ? "" : failmsg);
  return condition;
}


/// Init Image library.  (Call once!)
/// Currently, simply calibrate instrumentation and set names of counters.
void ImageInit(void) { ///
  InstrCalibrate();
  InstrName[0] = "pixmem"; // InstrCount[0] will count pixel array acesses
  InstrName[1] = "compare";  
  // Name other counters here...
  
}

// Macros to simplify accessing instrumentation counters:
#define PIXMEM InstrCount[0]
// Add more macros here...
#define compare InstrCount[1]


// TIP: Search for PIXMEM or InstrCount to see where it is incremented!


/// Image management functions

/// Create a new black image.
///   width, height : the dimensions of the new image.
///   maxval: the maximum gray level (corresponding to white).
/// Requires: width and height must be non-negative, maxval > 0.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCreate(int width, int height, uint8 maxval) { ///
  assert (width >= 0);
  assert (height >= 0);
  assert (0 < maxval && maxval <= PixMax);
  // Insert your code here!

  Image newImage = malloc(sizeof(struct image));              //Aloca memória para uma nova imagem
  if(newImage == NULL){                                       //Verifica se a alocação de memória para a imagem foi bem-sucedida
    errCause = "Memory allocation failed";                    //Defenição das mensagens de erro
    errno = 12;
    return NULL;                                              //Retorna NULL para indicar falha
  }

  newImage->width = width;                                    // Atribui valores aos membros da estrutura da imagem 
  newImage->height = height;                                  // Atribui valores aos membros da estrutura da imagem 
  newImage->maxval = maxval;                                  // Atribui valores aos membros da estrutura da imagem 

  newImage->pixel =calloc(width * height, sizeof(uint8_t));   //Aloca memória para os dados dos pixels
  if(newImage->pixel == NULL){                                //Verifica se a alocação de memória para os pixels foi bem-sucedida 
    errCause = "Memory alloction failed";                     //Defenição das mensagens de erro
    errno = 12;
    free(newImage);                                           //Liberta a estrutura de imagem previamente alocada
    return NULL;                                              //Retorna NULL para indicar falha
  }  
  return newImage;                                            //Retorna a imagem criada
}

/// Destroy the image pointed to by (*imgp).
///   imgp : address of an Image variable.
/// If (*imgp)==NULL, no operation is performed.
/// Ensures: (*imgp)==NULL.
/// Should never fail, and should preserve global errno/errCause.
void ImageDestroy(Image* imgp) { ///
  assert (imgp != NULL);                                      //Verifica se o ponteiro para a imagem não é nulo
  // Insert your code here!
  if (*imgp != NULL) {                                        //Verifica se  o ponteiro para a estrutura de imagem não é nulo
    
    free((*imgp)->pixel);                                     //Liberta a memória dos dados dos pixels

    
    free(*imgp);                                              //Liberta a memória da estrutura da imagem

    
    *imgp = NULL;                                             //Define o ponteiro como nulo para indicar que a imagem foi destruída
  }
}


/// PGM file operations

// See also:
// PGM format specification: http://netpbm.sourceforge.net/doc/pgm.html

// Match and skip 0 or more comment lines in file f.
// Comments start with a # and continue until the end-of-line, inclusive.
// Returns the number of comments skipped.
static int skipComments(FILE* f) {
  char c;
  int i = 0;
  while (fscanf(f, "#%*[^\n]%c", &c) == 1 && c == '\n') {
    i++;
  }
  return i;
}

/// Load a raw PGM file.
/// Only 8 bit PGM files are accepted.
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageLoad(const char* filename) { ///
  int w, h;
  int maxval;
  char c;
  FILE* f = NULL;
  Image img = NULL;

  int success = 
  check( (f = fopen(filename, "rb")) != NULL, "Open failed" ) &&
  // Parse PGM header
  check( fscanf(f, "P%c ", &c) == 1 && c == '5' , "Invalid file format" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &w) == 1 && w >= 0 , "Invalid width" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d ", &h) == 1 && h >= 0 , "Invalid height" ) &&
  skipComments(f) >= 0 &&
  check( fscanf(f, "%d", &maxval) == 1 && 0 < maxval && maxval <= (int)PixMax , "Invalid maxval" ) &&
  check( fscanf(f, "%c", &c) == 1 && isspace(c) , "Whitespace expected" ) &&
  // Allocate image
  (img = ImageCreate(w, h, (uint8)maxval)) != NULL &&
  // Read pixels
  check( fread(img->pixel, sizeof(uint8), w*h, f) == w*h , "Reading pixels" );
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (!success) {
    errsave = errno;
    ImageDestroy(&img);
    errno = errsave;
  }
  if (f != NULL) fclose(f);
  return img;
}

/// Save image to PGM file.
/// On success, returns nonzero.
/// On failure, returns 0, errno/errCause are set appropriately, and
/// a partial and invalid file may be left in the system.
int ImageSave(Image img, const char* filename) { ///
  assert (img != NULL);
  int w = img->width;
  int h = img->height;
  uint8 maxval = img->maxval;
  FILE* f = NULL;

  int success =
  check( (f = fopen(filename, "wb")) != NULL, "Open failed" ) &&
  check( fprintf(f, "P5\n%d %d\n%u\n", w, h, maxval) > 0, "Writing header failed" ) &&
  check( fwrite(img->pixel, sizeof(uint8), w*h, f) == w*h, "Writing pixels failed" ); 
  PIXMEM += (unsigned long)(w*h);  // count pixel memory accesses

  // Cleanup
  if (f != NULL) fclose(f);
  return success;
}


/// Information queries

/// These functions do not modify the image and never fail.

/// Get image width
int ImageWidth(Image img) { ///
  assert (img != NULL);
  return img->width;
}

/// Get image height
int ImageHeight(Image img) { ///
  assert (img != NULL);
  return img->height;
}

/// Get image maximum gray level
int ImageMaxval(Image img) { ///
  assert (img != NULL);
  return img->maxval;
}

/// Pixel stats
/// Find the minimum and maximum gray levels in image.
/// On return,
/// *min is set to the minimum gray level in the image,
/// *max is set to the maximum.
void ImageStats(Image img, uint8* min, uint8* max) { ///
  assert (img != NULL);
  // Insert your code here!
  if (img->width <= 0 || img->height <= 0) {
    fprintf(stderr, "Invalid image dimensions for stats calculation\n");
    return;
  }
  *min = *max = img->pixel[0];

  for (int i = 1; i < img->width * img->height; i++) {
    if (img->pixel[i] < *min) {
      *min = img->pixel[i];
    }
    if (img->pixel[i] > *max) {
      *max = img->pixel[i];
    }
  }
}

/// Check if pixel position (x,y) is inside img.
int ImageValidPos(Image img, int x, int y) { ///
  assert (img != NULL);
  return (0 <= x && x < img->width) && (0 <= y && y < img->height);
}

/// Check if rectangular area (x,y,w,h) is completely inside img.
int ImageValidRect(Image img, int x, int y, int w, int h) { ///
  //Verifica se o ponteiro para a imagem não é nulo
  assert (img != NULL);
  // Insert your code here!
  //Verifica se o ponto de partida está dentro dos limites da imagem
  if (x < 0 || y < 0 || x >= img->width || y >= img->height) {
        //Não é válido
        return 0; 
    }

    //Verifica se o retângulo se estende além dos extremos 
    if (x + w > img->width || y + h > img->height) {
        //Não é válido, o retângulo passa além das extremidades da imagem
        return 0;  
    }

    //Se todas as condições forem satisfeitas, o retângulo é válido
    return 1;

}

/// Pixel get & set operations

/// These are the primitive operations to access and modify a single pixel
/// in the image.
/// These are very simple, but fundamental operations, which may be used to 
/// implement more complex operations.

// Transform (x, y) coords into linear pixel index.
// This internal function is used in ImageGetPixel / ImageSetPixel. 
// The returned index must satisfy (0 <= index < img->width*img->height)
//Gera o índice correspondente à posição(x,y) na matriz unidimensional
static inline int G(Image img, int x, int y) {
  int index;
  // Insert your code here!

  assert(0 <= x && x < img->width);                         //Garante que x está dentro dos limites da largura da imagem
  assert(0 <= y && y < img->height);                        //Garante que y está dentro dos limites da altura da imagem
  index = y * img->width + x;                               //Calcula o índice na matriz unidimensional
  assert (0 <= index && index < img->width*img->height);    //Garante que o índice calculado está dentro dos limites da matriz
  return index;
}


uint8 ImageGetPixel(Image img, int x, int y) {              //Obtém o pixel na posição(x,y) para um novo nível
  assert (img != NULL);                                     //Verifica se o ponteiro para a imagem não é nulo
  assert (ImageValidPos(img, x, y));                        //Verifica se a posição (x,y) é válida dentro da imagem
  PIXMEM += 1;                                              //incrementa o contador de acesso ao pixel
  return img->pixel[G(img, x, y)];                          //Retorna o valor do pixel na posição (x, y)
} 


void ImageSetPixel(Image img, int x, int y, uint8 level) {  //Define o pixel na posição (x, y) para um novo nível.
  assert (img != NULL);                                     //Verifica se o ponteiro para a imagem não é nulo
  assert (ImageValidPos(img, x, y));                        //Verifica se a posição (x, y) é válida dentro da imagem
  PIXMEM += 1;                                              //Incrementa o contador de acesso ao pixel    
  img->pixel[G(img, x, y)] = level;                         //Define o valor do pixel na posição (x, y) para o novo nível
} 


/// Pixel transformations

/// These functions modify the pixel levels in an image, but do not change
/// pixel positions or image geometry in any way.
/// All of these functions modify the image in-place: no allocation involved.
/// They never fail.


/// Transform image to negative image.
/// This transforms dark pixels to light pixels and vice-versa,
/// resulting in a "photographic negative" effect.
void ImageNegative(Image img) {                                   //Inverte os valores dos pixels na imagem
  assert (img != NULL);                                           //Verifica se o ponteiro para a imagem não é nulo
  // Insert your code here!
  for(int i=0; i<img->width * img->height;i++){                   //Itera sobre todos os pixels na imagem
    img->pixel[i]=img->maxval -img->pixel[i];                     //Calcula o negativo de cada pixel em relação ao valor máximo
  }
}

/// Apply threshold to image.
/// Transform all pixels with level<thr to black (0) and
/// all pixels with level>=thr to white (maxval).
void ImageThreshold(Image img, uint8 thr) {                       
  assert (img != NULL);                                          //Verifica se o ponteiro para a imagem não é nulo
  // Insert your code here!
   for (int i = 0; i < img->width * img->height; i++) {          //Itera sobre todos os pixels na imagem
    if (img->pixel[i] < thr) {                                   //Verifica se o valor do pixel é inferior ao limiar
      img->pixel[i] = 0;                                         //Define para preto
    } else {
      img->pixel[i] = img->maxval;                               //Define para branco
    }
  }
}

/// Brighten image by a factor.
/// Multiply each pixel level by a factor, but saturate at maxval.
/// This will brighten the image if factor>1.0 and
/// darken the image if factor<1.0.
void ImageBrighten(Image img, double factor) {                        //Aumenta o brilho da imagem multiplicando cada pixel por um fator
  assert (img != NULL);                                               //Verifica se o ponteiro para a imagem não é nulo
  assert (factor >= 0.0);                                             //Verifica se o fator de aumento de brilho é não negativo
  // Insert your code here!
  

  

  for (int i = 0; i < img->width * img->height; i++) {                //Itera através de cada pixel na imagem

    double intensity = img->pixel[i]; 

    
    intensity *= factor;                                              //Multiplica a intensidade pelo fator

    
    if (intensity > img->maxval) {                                    //Satura em maxval
        img->pixel[i]=img->maxval;                                    //intensidade = img->maxval;
    }
    else{
      img->pixel[i] = (uint8) (intensity+0.5);                        //Atualiza o valor do pixel para a intensidade calculada, arredondando para o inteiro mais próximo
    }
  }
}


/// Geometric transformations

/// These functions apply geometric transformations to an image,
/// returning a new image as a result.
/// 
/// Success and failure are treated as in ImageCreate:
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.

// Implementation hint: 
// Call ImageCreate whenever you need a new image!

/// Rotate an image.
/// Returns a rotated version of the image.
/// The rotation is 90 degrees anti-clockwise.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.


Image ImageRotate(Image img) {                                        //Rotaciona a imagem 90 graus no sentido anti-horário
  //Verifica se o ponteiro para a imagem não é nulo                                        
  assert (img != NULL);
  // Insert your code here!
  //Cria uma nova imagem com largura e altura trocadas
  Image rotatedImg = ImageCreate(img->height, img->width, img->maxval);

  // Itera sobre cada pixel na imagem original
  for (int i = 0; i < img->height; i++) {
      for (int j = 0; j < img->width; j++) {
          // Rotaciona os pixels 90 graus no sentido anti-horário e copia para a nova imagem
          rotatedImg->pixel[(rotatedImg->height - 1 - j) * rotatedImg->width + i] = img->pixel[i * img->width + j];
      }
  }
  // Retorna a nova imagem rotacionada
  return rotatedImg;
}

/// Mirror an image = flip left-right.
/// Returns a mirrored version of the image.
/// Ensures: The original img is not modified.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageMirror(Image img) {                                                                          //Espelha a imagem horizontalmente
  assert (img != NULL);                                                                                 //Verifica se o ponteiro para a imagem não é nulo
  // Insert your code here!
  Image mirroredImg = ImageCreate(img->width, img->height, img->maxval);                                //Cria uma nova imagem espelhada com a mesma largura e altura 

  //Itera sobre cada linha da imagem original
  for (int i = 0; i < img->height; i++) {
      //Itera sobre cada pixel na linha, espelhando horizontalmente
      for (int j = 0; j < img->width; j++) {
          //Copia o pixel da imagem original para a imagem espelhada
          mirroredImg->pixel[i * img->width + (img->width - 1 - j)] = img->pixel[i * img-> width + j];
      }
  }

  return mirroredImg;                                                                                   //Retorna a nova imagem espelhada horizontalmente

}

/// Crop a rectangular subimage from img.
/// The rectangle is specified by the top left corner coords (x, y) and
/// width w and height h.
/// Requires:
///   The rectangle must be inside the original image.
/// Ensures:
///   The original img is not modified.
///   The returned image has width w and height h.
/// 
/// On success, a new image is returned.
/// (The caller is responsible for destroying the returned image!)
/// On failure, returns NULL and errno/errCause are set accordingly.
Image ImageCrop(Image img, int x, int y, int w, int h) {                                    //Corta uma região específica da imagem
  assert (img != NULL);                                                                     //Verifica se o ponteiro para a imagem não é nulo
  assert (ImageValidRect(img, x, y, w, h));                                                 //Verifica se a região de corte é válida dentro da imagem
  // Insert your code here!
  Image croppedImg = ImageCreate(w, h, img->maxval);                                        //Cria uma nova imagem cortada com a largura e altura especificadas

  //Itera sobre cada linha da região de corte
  for (int i = 0; i < h; i++) {
      //Itera sobre cada pixel na linha da região de corte
      for (int j = 0; j < w; j++) {
          //Copia o pixel da imagem original para a imagem cortada 
          croppedImg->pixel[i * w + j] = img->pixel[(y + i) * img->width + (x + j)];
      }
  }

  return croppedImg;                                                                        //Retorna a nova imagem cortada

}


/// Operations on two images

/// Paste an image into a larger image.
/// Paste img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
void ImagePaste(Image img1, int x, int y, Image img2) {                                   //Cola uma imagem sobre outra em uma posição específica
  assert (img1 != NULL);                                                                  //Verifica se os ponteiros para as imagens não são nulos
  assert (img2 != NULL);                                                                  
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));                         //Verifica se a região de colagem é válida dentro da imagem de destino (img1)
  // Insert your code here!
  //Itera sobre cada linha da imagem a ser colada (img2)
  for (int i = 0; i < img2->height; i++) {
      //Itera sobre cada pixel na linha da imagem a ser colada (img2)
      for (int j = 0; j < img2->width; j++) {
          //Copia o pixel da imagem a ser colada (img2) para a imagem de destino (img1)
          img1->pixel[(y + i) * img1->width + (x + j)] = img2->pixel[i * img2->width + j];
      }
  }

}

/// Blend an image into a larger image.
/// Blend img2 into position (x, y) of img1.
/// This modifies img1 in-place: no allocation involved.
/// Requires: img2 must fit inside img1 at position (x, y).
/// alpha usually is in [0.0, 1.0], but values outside that interval
/// may provide interesting effects.  Over/underflows should saturate.
void ImageBlend(Image img1, int x, int y, Image img2, double alpha) {                       //Realiza uma mistura (blend) entre duas imagens em uma posição específica
  //Verifica se os ponteiros para as imagens não são nulos
  assert (img1 != NULL);
  assert (img2 != NULL);
  //Verifica se a região de mistura é válida dentro da imagem de destino (img1)
  assert (ImageValidRect(img1, x, y, img2->width, img2->height));
  // Insert your code here!
  //Itera sobre cada linha da imagem a ser misturada (img2)
  for (int i = 0; i < img2->height; i++) {
      //Itera sobre cada pixel na linha da imagem a ser misturada (img2)
      for (int j = 0; j < img2->width; j++) {
        //Calcula o pixel misturado usando a fórmula de mistura com o fator alpha
        uint8 blended_Pixel = (uint8)(alpha * ImageGetPixel (img2,j,i) + (1.0 -alpha)* ImageGetPixel(img1,x+j,y+i)+0.5);
          //Garante que o valor do pixel misturado esteja dentro dos limites válidos
          if(blended_Pixel<0)
            ImageSetPixel(img1,x+j,y+i,0);
          else if (blended_Pixel > img1->maxval)
            ImageSetPixel(img1,x+j,y+i,img1->maxval);
          else
            ImageSetPixel(img1,x+j,y+i,blended_Pixel);
      }
  }
}

/// Compare an image to a subimage of a larger image.
/// Returns 1 (true) if img2 matches subimage of img1 at pos (x, y).
/// Returns 0, otherwise.
int ImageMatchSubImage(Image img1, int x, int y, Image img2) {                  //// Verifica se uma subimagem é correspondente em uma posição específica dentro de outra imagem
  //Verifica se os ponteiros para as imagens não são nulos
  assert (img1 != NULL);
  assert (img2 != NULL);
  //Verifica se a posição de início é válida dentro da imagem de destino (img1)
  assert (ImageValidPos(img1, x, y)); 
  // Insert your code here!
  //Itera sobre cada pixel da subimagem (img2)
  for (int i = 0; i < img2->height; i++) {
      //Itera sobre cada pixel na linha da subimagem (img2)
      for (int j = 0; j < img2->width; j++) {
          //Conta o número de comparações
          compare++;
          //Verifica se a posição é válida dentro da imagem de destino (img1) e se os pixels são diferentes
          if (ImageValidPos(img1, x + j, y + i) &&
              img1->pixel[(y + i) * img1->width + (x + j)] != img2->pixel[i * img2->width + j]) {
              return 0; //Retorna 0 se um pixel diferente for encontrado
          }
      }
  }
  //Retorna 1 se todos os pixels coincidirem
  return 1; 
}

/// Locate a subimage inside another image.
/// Searches for img2 inside img1.
/// If a match is found, returns 1 and matching position is set in vars (*px, *py).
/// If no match is found, returns 0 and (*px, *py) are left untouched.
int ImageLocateSubImage(Image img1, int* px, int* py, Image img2) {         //Localiza a posição de uma subimagem dentro de outra imagem
  //Verifica se os ponteiros para as imagens não são nulos
  assert (img1 != NULL);
  assert (img2 != NULL);
  // Insert your code here!
  //Itera sobre cada posição possível na imagem de destino (img1)
  for (int i = 0; i < img1->height; i++) {
      for (int j = 0; j < img1->width; j++) {
          //Conta o número de comparações
          compare++;
          //Verifica se a subimagem (img2) coincide com a posição atual na imagem de destino (img1)
          if (ImageMatchSubImage(img1, j, i, img2)) {
              //Se coincidir, atualiza as coordenadas e retorna 1
              *px = j;
              *py = i;
              return 1; 
          }
      }
  }
  //Retorna 0 se a subimagem não for encontrada em nenhuma posição
  return 0; 
}


/// Filtering

/// Blur an image by a applying a (2dx+1)x(2dy+1) mean filter.
/// Each pixel is substituted by the mean of the pixels in the rectangle
/// [x-dx, x+dx]x[y-dy, y+dy].
/// The image is changed in-place.
void ImageBlur(Image img, int dx, int dy) {                             //Aplica um efeito de desfoque (blur) em uma imagem
  // Insert your code here!
  assert(img != NULL);                                                  //Verifica se o ponteiro para a imagem não é nulo e se os parâmetros de desfoque são válidos
  assert(dx >= 0 && dy >= 0);

  //Cria uma nova imagem para armazenar o resultado do desfoque
  Image newImage = ImageCreate(img->width, img->height, img->maxval);

  //Verifica se a criação da nova imagem foi bem-sucedida
  if (newImage == NULL) {
    return;
  }

  //Itera sobre cada pixel na imagem original
  for (size_t i = 0; i < img->width; i++) {
    for (size_t j = 0; j < img->height; j++) {
      
      double sum = 0;
      double count = 0;
      int startX = i - dx;
      int startY = j - dy;

      //Garante que as coordenadas de início não sejam negativas
      if (startX < 0) {
        startX = 0;
      }
      if (startY < 0) {
        startY = 0;
      }

      //Itera sobre uma região definida pelos parâmetros de desfoque
      for (size_t k = startX; k <= i + dx; k++) {
        for (size_t l = startY; l <= j + dy; l++) {
          //Verifica se a posição é válida na imagem original
          if (ImageValidPos(img, k, l)) {
            //Soma os valores dos pixels na região para calcular a média
            sum += ImageGetPixel(img, k, l);
            count++;
          }
        }
      }

      //Calcula a média e arredonda para o valor mais próximo
      uint8 mean = (uint8)round(sum / count);
      //Garante que o valor médio esteja dentro dos limites válidos
      if (mean > img->maxval) {
        mean = img->maxval;
      } else if (mean < 0) {
        mean = 0;
      }

      // Define o pixel na nova imagem com o valor médio calculado
      ImageSetPixel(newImage, i, j, mean);
    }
  }
  // Copia os pixels da nova imagem para a imagem original
  for (size_t i = 0; i < img->width; i++) {
    for (size_t j = 0; j < img->height; j++) {
      ImageSetPixel(img, i, j, ImageGetPixel(newImage, i, j));
    }
  }
  //Libera a memória alocada para a nova imagem
  ImageDestroy(&newImage);
}

