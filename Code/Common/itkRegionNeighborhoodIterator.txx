/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkRegionNeighborhoodIterator.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

  =========================================================================*/
namespace itk {
  
template<class TPixel, unsigned int VDimension>
Neighborhood<TPixel, VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>
::GetNeighborhood()
{
  NeighborhoodType ans;
  typename NeighborhoodType::Iterator ans_it;
  Iterator this_it;
  const Iterator _end = this->end();

  ans.SetRadius( this->GetRadius() );

  for (ans_it = ans.begin(), this_it = this->begin();
       this_it < _end; ans_it++, this_it++)
    {
      *ans_it = **this_it;
    }

  return ans;
}

template<class TPixel, unsigned int VDimension>
void
RegionNeighborhoodIterator<TPixel, VDimension>
::SetNeighborhood(NeighborhoodType &N)
{
  Iterator this_it;
  const Iterator _end = this->end();
  typename NeighborhoodType::Iterator N_it;
  
  for (this_it = this->begin(); this_it < _end; this_it++, N_it++)
    {
      **this_it = *N_it;
    }

}
  
template<class TPixel, unsigned int VDimension>
void RegionNeighborhoodIterator<TPixel, VDimension>
::PrintSelf()  
{          
  int i;   
  //NeighborhoodBase<TPixel, VDimension>::PrintSelf();
  std::cout << "RegionNeighborhoodIterator" << std::endl;
  std::cout << "        this = " << this << std::endl;
  std::cout << "this->size() = " << this->size() << std::endl;
  std::cout << "    m_Bound = { ";
  for (i = 0; i<VDimension; i++) std::cout << m_Bound[i] << " ";
  std::cout << "}" << std::endl;
  std::cout << "     m_Loop = { ";
  for (i = 0; i<VDimension; i++) std::cout << m_Loop[i] << " ";
  std::cout << "}" << std::endl;
}
  
template<class TPixel, unsigned int VDimension>
void RegionNeighborhoodIterator<TPixel, VDimension>
::SetBound(const SizeType& size)
{
  const unsigned long *offset     = m_Image->GetOffsetTable();
  Size<VDimension> bufferSize = m_Image->GetBufferedRegion().GetSize();

  // Set the bounds and the wrapping offsets
  for (int i=0; i<VDimension; ++i)
    {
      m_Bound[i]      = m_StartIndex[i]+size[i];
      m_WrapOffset[i] = (bufferSize[i] - (m_Bound[i] - m_StartIndex[i]))
                        * offset[i];
    }  
}

template<class TPixel, unsigned int VDimension>
typename RegionNeighborhoodIterator<TPixel, VDimension>::ScalarValueType
RegionNeighborhoodIterator<TPixel, VDimension>
::InnerProduct(std::valarray<TPixel> &v)
{
  ScalarValueType sum = NumericTraits<ScalarValueType>::Zero;
  TPixel *it;
  Iterator this_it;
  const TPixel *_end = &(v[v.size()]);
  
  for (it = &(v[0]), this_it = this->begin(); it < _end; ++it, ++this_it)
    {
      sum += *it * **this_it;
    }
  return sum;
}

template<class TPixel, unsigned int VDimension>
typename RegionNeighborhoodIterator<TPixel, VDimension>::ScalarValueType
RegionNeighborhoodIterator<TPixel, VDimension>
::InnerProduct(std::valarray<typename RegionNeighborhoodIterator<TPixel,
               VDimension>::ScalarValueType> &v,
               VectorComponentDataAccessor<TPixel,
               typename RegionNeighborhoodIterator<TPixel,
               VDimension>::ScalarValueType> & accessor)
{
  ScalarValueType sum  = NumericTraits<ScalarValueType>::Zero; 
  
  ScalarValueType *it;
  Iterator this_it;

  const ScalarValueType *itEnd = &(v[v.size()]);
  for (it = &(v[0]), this_it = this->begin(); it < itEnd;
       ++it, ++this_it)
    {
      sum += *it * accessor.Get(**this_it);
    }
  
  return sum;
}


template<class TPixel, unsigned int VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>::ScalarValueType
RegionNeighborhoodIterator<TPixel, VDimension>
::SlicedInnerProduct(const std::slice &s, std::valarray<TPixel> &v)
{
  ScalarValueType sum = NumericTraits<ScalarValueType>::Zero;
  TPixel *it;
  typename RegionNeighborhoodIterator::SliceIteratorType slice_it(this, s);

  slice_it[0];
  const TPixel *itEnd = &(v[v.size()]);
  for (it = &(v[0]); it < itEnd; ++it, ++slice_it)
    {
      sum += *it * **slice_it;
    }

  return sum;
  
}

template<class TPixel, unsigned int VDimension>
typename RegionNeighborhoodIterator<TPixel, VDimension>::ScalarValueType
RegionNeighborhoodIterator<TPixel, VDimension>
::SlicedInnerProduct(const std::slice& s, std::valarray<ScalarValueType> &v, 
               VectorComponentDataAccessor<TPixel, ScalarValueType>
               &accessor)
{
  ScalarValueType sum = NumericTraits<ScalarValueType>::Zero;
  ScalarValueType *it;
  typename Self::SliceIteratorType slice_it(this, s);

  slice_it[0];
  const ScalarValueType *itEnd = &(v[v.size()]);
  for (it = &(v[0]); it < itEnd; ++it, ++slice_it)
    {
      //sum += *it * *slice_it;
      sum += *it * accessor.Get(**slice_it);
    }

  return sum;
}

template<class TPixel, unsigned int VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>
::Begin() const
{
  //Copy the current iterator
  RegionNeighborhoodIterator it( *this );

  // Set the position to the m_BeginOffset
  it.SetLocation( this->m_StartIndex );

  return it;
}

template<class TPixel, unsigned int VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>
RegionNeighborhoodIterator<TPixel, VDimension>
::End() const
{
  IndexType endIndex;
  
  // Copy the current iterator
  RegionNeighborhoodIterator it( *this );

  // Calculate the end index
  for (int i = 0; i< VDimension; ++i)
    {
      endIndex.m_Index[i] = m_Bound[i] -1;
    }
  it.SetLocation( endIndex );

  ++it;
  return it;
}

} // namespace itk
