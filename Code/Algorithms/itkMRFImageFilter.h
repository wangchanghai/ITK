/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkMRFImageFilter.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef _itkMRFImageFilter_h
#define _itkMRFImageFilter_h

//#include "itkObject.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_matrix.h"

#include "itkImageToImageFilter.h"
#include "itkSupervisedClassifier.h"

namespace itk
{

/** \class MRFImageFilter
 * \brief Implementation of a labeller object that uses Markov Random Fields
 * to classify pixels in a 3-D data set.
 * 
 * This object classifies pixels based on a 3-D Markov Random Field (MRF) 
 * model.This implementation uses the maximum a posteriori (MAP) estimates
 * for modeling the MRF. The object traverses the data set and uses the model 
 * generated by the Gaussian classifier to gets the the distance between 
 * each pixel in the data set to a set of known classes, updates the 
 * distances by evaluating the influence of its neighboring pixels (based 
 * on a 3-D MRF model) and finally, classifies each pixel to the class 
 * which has the minimum distance to that pixel (taking the neighborhood 
 * influence under consideration).
 *
 * The a classified initial labeled image is needed. It is important
 * that the number of expected classes be set before calling the 
 * classifier. In our case we have used the GaussianSupervisedClassifer to
 * generate the initial labels. This classifier requires the user to 
 * ensure that an appropriate training image set be provided. See the 
 * documentation of the classifier class for more information.
 *
 * The influence of a three-dimensional neighborhood on a given pixel's
 * classification (the MRF term) is computed by calculating a weighted
 * sum of number of class labels in a three dimensional neighborhood.
 * The basic idea of this neighborhood influence is that if a large
 * number of neighbors of a pixel are of one class, then the current
 * pixel is likely to be of the same class.
 *
 * The dimensions of the 3-D neighborhood and values of the weighting 
 * parameters are either specified by the user through the beta 
 * parameter or a default weighting table is generated during object 
 * construction. The following table shows an example of a 3x3x3 
 * neighborhood and the weighting values used. A 3 x 3 x 3 kernel
 * is used where each value is a nonnegative parameter, which encourages 
 * neighbors to be of the same class. In this example, the influence of
 * the pixels in the same slice is assigned a weight 1.7, the influence
 * of the pixels in the same location in the previous and next slice is 
 * assigned a weight 1.5, while a weight 1.3 is assigned to the influence of 
 * the north, south, east, west and diagonal pixels in the previous and next 
 * slices. 
 * \f[\begin{tabular}{ccc}
 *  \begin{tabular}{|c|c|c|}
 *   1.3 & 1.3 & 1.3 \\
 *   1.3 & 1.5 & 1.3 \\
 *   1.3 & 1.3 & 1.3 \\
 *  \end{tabular} &
 *  \begin{tabular}{|c|c|c|}
 *   1.7 & 1.7 & 1.7 \\
 *   1.7 & 0 & 1.7 \\
 *   1.7 & 1.7 & 1.7 \\
 *  \end{tabular} &
 *  \begin{tabular}{|c|c|c|}
 *   1.3 & 1.3 & 1.3 \\
 *   1.5 & 1.5 & 1.3 \\
 *   1.3 & 1.3 & 1.3 \\
 *  \end{tabular} \\
 * \end{tabular}\f]
 *
 * For minimization of the MRF labeling function the MinimizeFunctional
 * virtual method is called. For our current implementation we use the
 * the iterated conditional modes (ICM) algorithm described by Besag in the
 * paper ``On the Statistical Analysis of Dirty Pictures'' in J. Royal Stat.
 * Soc. B, Vol. 48, 1986. 
 *
 * In each iteration, the algorithm visits each pixel in turn and 
 * determines whether to update its classification by computing the influence
 * of the classification of the pixel's neighbors and of the intensity data.
 * On each iteration after the first, we reexamine the classification of a 
 * pixel only if the classification of some of its neighbors has changed
 * in the previous iteration. The pixels' classification is updated using a 
 * synchronous scheme (iteration by iteration) until the error reaches
 * less than the threshold or the number of iteration exceed the maximum set
 * number of iterations. 
 *
 * \ingroup MRFFilters
 */
template <class TInputImage, class TClassifiedImage>
class ITK_EXPORT MRFImageFilter : 
  public ImageToImageFilter<TInputImage,TClassifiedImage>
{
public:       
  /** Standard class typedefs. */
  typedef MRFImageFilter   Self;
  typedef ImageToImageFilter<TInputImage,TClassifiedImage> Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MRFImageFilter,Object);

  /** Type definition for the input image. */
  typedef typename TInputImage::Pointer              InputImagePointer;  

  /** Type definition for the input image pixel type. */
  typedef typename TInputImage::PixelType            InputPixelType;

  /** Type definitions for the training image. */
  typedef typename TClassifiedImage::Pointer         TrainingImagePointer;

  /** Type definitions for the training image pixel type. */
  typedef typename TClassifiedImage::PixelType       TrainingPixelType;

  /** Type definitions for the labelled image.
   * It is derived from the training image. */
  typedef typename TClassifiedImage::Pointer         LabelledImagePointer;
      
  /** Type definitions for the classified image pixel type.
   * It has to be the same type as the training image. */
  typedef typename TClassifiedImage::PixelType       LabelledPixelType;

  /** Type definition for the classified image index type. */
  typedef typename TClassifiedImage::IndexType       LabelledImageIndexType;

  /** Type definition for the classified image offset type. */
  typedef typename TClassifiedImage::OffsetType       LabelledImageOffsetType;

  /** Type definitions for classifier to be used for the MRF lavbelling. */
  typedef Classifier<TInputImage,TClassifiedImage> ClassifierType;
  typedef typename TInputImage::PixelType      InputImagePixelType;
  typedef typename TClassifiedImage::PixelType TrainingImagePixelType;
  typedef typename TClassifiedImage::PixelType LabelledImagePixelType;
  typedef ImageRegionIteratorWithIndex< TInputImage > InputImageIterator;
  typedef ImageRegionIteratorWithIndex< TClassifiedImage > 
          LabelledImageIterator;
  typedef typename TInputImage::PixelType    InputImageVectorType;

  /** Set the image required for training type classifiers. */
  void SetTrainingImage(TrainingImagePointer image);

  /** Get the traning image.  */
  TrainingImagePointer GetTrainingImage();


  /** Set the pointer to the classifer being used. */
  void SetClassifier( typename ClassifierType::Pointer ptrToClassifier );

  /** Set/Get the number of classes. */
  itkSetMacro(NumberOfClasses, unsigned int);
  itkGetMacro(NumberOfClasses, unsigned int);

  /** Set/Get the number of iteration of the Iterated Conditional Mode
   * (ICM) algorithm. A default value is set at 50 iterations. */
  itkSetMacro(MaximumNumberOfIterations, unsigned int);
  itkGetMacro(MaximumNumberOfIterations, unsigned int);

  /** Set/Get the error tollerance level which is used as a threshold
   * to quit the iterations */
  itkSetMacro(ErrorTolerance, double);
  itkGetMacro(ErrorTolerance, double);

  /** Set/Get the weighting parameters (Beta Matrix). A default 3 x 3 x 3 
   * matrix is provided. However, the user is allowed to override it
   * with their choice of weights for a 3 x 3 x 3 matrix. */
  virtual void SetBeta( double* );
  double* GetBeta()
    { return m_Beta3x3x3; }

  /** Set the weighting parameters (Beta Matrix). This is an overloaded
   * function allowing the users to set the Beta Matrix by providing a 
   * a 1D array of weights. Current implementation supports only a 
   * 3 x 3 x 3 kernel. The labeler needs to be extended for a different
   * kernel size. */
  virtual void SetBeta( double *BetaMatrix, unsigned int kernelSize );
  virtual void SetBeta( vnl_vector<double> BetaMatrix );
      
protected:
  MRFImageFilter();
  ~MRFImageFilter();
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Allocate memory for labelled images. */
  void Allocate();

  /** Apply MRF Classifier. In this example the images are labelled using
   * Iterated Conditional Mode algorithm by J. Besag, "On statistical
   * analysis of dirty pictures," J. Royal Stat. Soc. B, vol. 48,
   * pp. 259-302, 1986. */
  virtual void ApplyMRFImageFilter();  

  /** Minimization algorithm to be used. */
  virtual void MinimizeFunctional();

  virtual void GenerateData();
  virtual void GenerateInputRequestedRegion();
  virtual void EnlargeOutputRequestedRegion( DataObject * );
  virtual void GenerateOutputInformation();

private:            
  MRFImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
  typedef typename TInputImage::SizeType InputImageSizeType;
 
  unsigned int           m_NumberOfClasses;
  unsigned int           m_MaximumNumberOfIterations;
  unsigned int           m_KernelSize;
  unsigned int           *m_LabelStatus;
  
  double                 m_ErrorTolerance;
  double                 *m_ClassProbability; //Class liklihood
  double                 *m_Beta3x3x3;

  /** Pointer to the classifier to be used for the MRF lavbelling. */
  typename ClassifierType::Pointer m_ClassifierPtr;


  int                    m_ErrorCounter;
  int                    *m_Offset;
  int                    m_kWidth;
  int                    m_kHeight;
  int                    m_kDepth;
  int                    m_imgWidth;
  int                    m_imgHeight;
  int                    m_imgDepth;

  int                    *m_WidthOffset;
  int                    *m_HeightOffset;
  int                    *m_DepthOffset;

  //Function implementing the ICM algorithm to label the images
  void ApplyICMLabeller();

}; // class MRFImageFilter


} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMRFImageFilter.txx"
#endif



#endif

