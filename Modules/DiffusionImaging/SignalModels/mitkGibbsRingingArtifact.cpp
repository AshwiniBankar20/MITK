/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

template< class ScalarType >
GibbsRingingArtifact< ScalarType >::GibbsRingingArtifact()
    : m_KspaceCropping(0.1)
{

}

template< class ScalarType >
GibbsRingingArtifact< ScalarType >::~GibbsRingingArtifact()
{

}

template< class ScalarType >
void GibbsRingingArtifact< ScalarType >::AddArtifact(typename ComplexSliceType::Pointer slice)
{
    itk::ImageRegion<2> region = slice->GetLargestPossibleRegion();
    itk::ImageRegionIterator<ComplexSliceType> it(slice, region);
    int x = std::ceil((double)region.GetSize()[0]*m_KspaceCropping);
    int y = std::ceil((double)region.GetSize()[1]*m_KspaceCropping);
    while(!it.IsAtEnd())
    {
        typename ComplexSliceType::IndexType idx = it.GetIndex();
        if (idx[0]<x)
            it.Set(vcl_complex<double>(0,0));
        if (idx[0]>region.GetSize()[0]-x)
            it.Set(0);
        if (idx[1]>region.GetSize()[1]-y)
            it.Set(0);
        if (idx[1]<y)
            it.Set(0);
        ++it;
    }
}
