/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <fstream>
#include <list>
#include <sstream>
#include <string>

#include <itkImage.h>
#include <itkTimeProbe.h>
#include <itkVector.h>

#include <itkImageToTreeFilter.h>
#include <itkITTFilterContext.h>
#include <itkProfileData.h>
#include <itkProfileGradientFinder.h>
#include <itkRefinementModelProcessor.h>
#include <itkRegistrationModelTree.h>
#include <itkRegistrationModelXMLReader.h>
#include <itkRegistrationModelXMLWriter.h>
#include <itkStartPointData.h>
#include <itkTransform.h>
#include <itkTranslationTransform.h>
#include <itkTreeToBinaryImageFilter.h>
#include <itkTubeSegmentModel.h>
#include <itkTubeSegmentModelGenerator.h>

// image type
typedef short                                         PixelType;
const unsigned int                                    DIMENSION = 3;

typedef double                                        ScalarType;

typedef itk::Image<PixelType, DIMENSION>              ImageType;
typedef ImageType::Pointer                            ImagePointer;
typedef ImageType::PointType                          PointType;
typedef ImageType::DirectionType                      DirectionType;

typedef itk::DefaultDynamicMeshTraits<PixelType, DIMENSION, DIMENSION,
    ScalarType, ScalarType, PixelType>
    MeshTraits;

typedef itk::RegistrationModel<PixelType, DIMENSION, MeshTraits>
    RegistrationModelType;
typedef RegistrationModelType::Pointer                RegistrationModelPointer;

typedef itk::RegistratedModel<PixelType, DIMENSION, MeshTraits>
    RegistratedModelType;
typedef RegistratedModelType::Pointer                 RegistratedModelPointer;

// tree type
typedef itk::RegistrationModelTree<RegistrationModelType>
OutputTreeType;
typedef OutputTreeType::TreeContainerPointer          OutputTreeContainerPointer;
typedef OutputTreeType::Pointer                       OutputTreePointer;

typedef short                                         OutputPixelType;
typedef itk::Image<OutputPixelType, DIMENSION>        OutputImageType;
typedef OutputImageType::Pointer                      OutputImagePointer;
typedef OutputImageType::PointType                    OutputImagePointType;

// test classes
typedef itk::ImageToTreeFilter<ImageType, OutputTreeType>
ImageToTreeFilterType;
typedef ImageToTreeFilterType::Pointer                ImageToTreeFilterPointer;
typedef itk::ITTFilterContext<ImageType, OutputTreeType>
FilterContextType;
typedef FilterContextType::Pointer                    FilterContextPointer;
typedef itk::StartPointData<ImageType>                StartPointDataType;
typedef StartPointDataType::Pointer                   StartPointDataPointer;
typedef StartPointDataType::VectorType                StartPointDataVectorType;

typedef itk::TubeSegmentModel<PixelType, DIMENSION, MeshTraits>
    TubeSegmentModelType;
typedef TubeSegmentModelType::Pointer                 TubeSegmentModelPointer;

typedef itk::RegistrationModelXMLWriter<TubeSegmentModelType>
RegistrationModelWriterType;
typedef RegistrationModelWriterType::Pointer          RegistrationModelWriterPointer;
typedef itk::RegistrationModelXMLReader<TubeSegmentModelType>
RegistrationModelReaderType;
typedef RegistrationModelReaderType::Pointer          RegistrationModelReaderPointer;

typedef TubeSegmentModelType::MeshType                MeshType;
typedef MeshType::Pointer                             MeshPointer;

typedef TubeSegmentModelType::PointSetType            PointSetType;
typedef PointSetType::Pointer                         PointSetPointer;
typedef PointSetType::PointType                       PointSetPointType;
typedef PointSetType::PointsContainer                 PointsContainerType;
typedef PointsContainerType::Iterator                 PointsIterator;
typedef PointsContainerType::Pointer                  PointsContainerPointer;
typedef PointsContainerType::ElementIdentifier        ElementIdentifier;
typedef PointSetType::PointDataContainer              PointDataContainerType;
typedef PointDataContainerType::Pointer               PointDataContainerPointer;

typedef itk::Transform<ScalarType>                    BaseTransformType;
typedef BaseTransformType::Pointer                    BaseTransformPointer;
typedef itk::TranslationTransform<ScalarType>         TransformType;
typedef TransformType::Pointer                        TransformPointer;
typedef TransformType::ParametersType                 TransformParamtersType;
typedef TransformType::OutputVectorType               TransformOutputVectorType;
typedef TransformType::InputPointType                 TransformInputPointType;
typedef TransformType::OutputPointType                TransformOutputPointType;

typedef itk::TreeToBinaryImageFilter<OutputTreeType, OutputImageType>
TreeToImageFilterType;
typedef TreeToImageFilterType::Pointer                TreeToImageFilterPointer;

typedef itk::ProfileGradientFinder<OutputImageType>   ProfileGradientFinderType;
typedef ProfileGradientFinderType::Pointer            ProfileGradientFinderPointer;

typedef ProfileGradientFinderType::ProfileDataType    ProfileDataType;
typedef ProfileDataType::Pointer                      ProfileDataPointer;

typedef itk::RefinementModelProcessor<RegistrationModelType, ImageType>
    RefinementModelProcessorType;
typedef RefinementModelProcessorType::Pointer         RefinementModelProcessorPointer;

typedef itk::PointSetToImageFilter<PointSetType, ImageType>
    PointSetToImageFilterType;
typedef PointSetToImageFilterType::Pointer            PointSetToImageFilterPointer;

typedef itk::TubeSegmentModelGenerator<PixelType>
    TubeSegmentModelGeneratorType;
typedef TubeSegmentModelGeneratorType::Pointer        TubeSegmentModelGeneratorPointer;

typedef itk::TubeSegmentModelRegistrator<PointSetType, ImageType, OutputTreeType>
    TubeSegmentModelRegistratorType;
typedef TubeSegmentModelRegistratorType::Pointer          TubeSegmentModelRegistratorPointer;

typedef itk::Vector<ScalarType>                       VectorType;


typedef std::list<int>                                ResultListType;

TubeSegmentModelPointer generateTubeSegment()
{
  PixelType pixelValue = 255;

  TubeSegmentModelPointer tubeSegment = TubeSegmentModelType::New();
  ElementIdentifier index = 0;
  PointSetPointer pointSet = PointSetType::New();
  PointsContainerPointer pointsContainer = PointsContainerType::New();
  PointDataContainerPointer pointDataContainer = PointDataContainerType::New();

  // init test data
  PointType startPoint;
  startPoint.Fill(0);
  tubeSegment->SetStartPoint(startPoint);
  tubeSegment->SetRotationPoint(startPoint);

  PointSetPointType point1; // 1, 1, 0
  point1[0] = 1;
  point1[1] = 1;
  point1[2] = 0;
  pointsContainer->InsertElement(index, point1);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  PointSetPointType point2; // 1, 0, 1
  point2[0] = 1;
  point2[1] = 0;
  point2[2] = 1;
  pointsContainer->InsertElement(index, point2);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  PointSetPointType point3; // 1, 0, 1
  point3[0] = 1;
  point3[1] = 1;
  point3[2] = 0;
  pointsContainer->InsertElement(index, point3);
  pointDataContainer->InsertElement(index, pixelValue);
  index++;

  pointSet->SetPoints(pointsContainer);
  pointSet->SetPointData(pointDataContainer);
  tubeSegment->SetPointSet(pointSet);

  PointSetPointer connectionPointSet = PointSetType::New();
  PointsContainerPointer connectionPointsContainer = PointsContainerType::New();
  index = 0;
  connectionPointsContainer->InsertElement(index, point1);
  connectionPointSet->SetPoints(connectionPointsContainer);
  tubeSegment->SetConnectionPoints(connectionPointSet);

  return tubeSegment;
}

PointType transformPoint(TransformPointer transform, TransformParamtersType parameters, PointType point)
{
  transform->SetParameters(parameters);

  TransformInputPointType inputPoint;
  for (unsigned int dim = 0; dim < DIMENSION; dim++)
  {
    inputPoint[dim] = point[dim];
  }
  TransformOutputPointType outputPoint;
  outputPoint = transform->TransformPoint(inputPoint);
  PointType transformedPoint;
  for (unsigned int dim = 0; dim < DIMENSION; dim++)
  {
    transformedPoint[dim] = outputPoint[dim];
  }
  return transformedPoint;
}

PointSetPointer transformPointSet(TransformPointer transform, TransformParamtersType parameters, PointSetPointer pointSet)
{
  transform->SetParameters(parameters);

  PointsContainerPointer transformedPoints = PointsContainerType::New();
  ElementIdentifier index = 0;

  PointsIterator pointsIter = pointSet->GetPoints()->Begin();
  PointsIterator lastPoint  = pointSet->GetPoints()->End();
  while(pointsIter != lastPoint)
  {
    PointSetPointType point = pointsIter.Value();
    TransformInputPointType inputPoint;
    for (unsigned int dim = 0; dim < DIMENSION; dim++)
    {
      inputPoint[dim] = point[dim];
    }
    TransformOutputPointType outputPoint;
    outputPoint = transform->TransformPoint(inputPoint);
    PointSetPointType transformedPoint;
    for (unsigned int dim = 0; dim < DIMENSION; dim++)
    {
      transformedPoint[dim] = outputPoint[dim];
    }
    transformedPoints->InsertElement(index, transformedPoint);

    ++pointsIter;
    ++index;
  }

  PointSetPointer transformedPointSet = PointSetType::New();
  transformedPointSet->SetPoints(transformedPoints);

  return transformedPointSet;
}

/****************************************************************
 * TEST: XML Writer
 ****************************************************************/
int testRegistrationModelXMLWriter()
{
  // TODO: add new point types
  std::cout << " *** Testing the RegistrationModelXMLWriter ***\n";

  TubeSegmentModelPointer tubeSegment = generateTubeSegment();
  PointType startPoint = tubeSegment->GetStartPoint();
  PointsContainerPointer pointsContainer = tubeSegment->GetPointSet()->GetPoints();

  RegistrationModelWriterPointer modelWriter = RegistrationModelWriterType::New();
  modelWriter->SetRegistrationModel(tubeSegment);
  modelWriter->SetFilename("test.xml");
  modelWriter->WriteFile();

  // compare to normal data
  std::ifstream file("test.xml");
  std::stringstream buffer;

  char charBuffer;
  while (file.get(charBuffer))
  {
    if (charBuffer != ' ' && charBuffer != '\n')
    {
      buffer.put(charBuffer);
    }
  }

  if (!file.eof() || buffer.bad())
  {
    std::cout << "Error reading the testfile.";
  }

  // TODO: find a better way to compare the strings
  std::string value = buffer.str();
  std::string expected = "<model><name>TubeSegment</name><startPoint><point><x>0</x><y>0</y><z>0</z></point></startPoint><rotationPoint><point><x>0</x><y>0</y><z>0</z></point></rotationPoint><connectionPoints><point><x>1</x><y>1</y><z>0</z></point></connectionPoints><points><point><x>1</x><y>1</y><z>0</z><value>255</value></point><point><x>1</x><y>0</y><z>1</z><value>255</value></point><point><x>1</x><y>1</y><z>0</z><value>255</value></point></points></model>";

  if (value != expected)
  {
    std::cout << "XML does not match!\n";
    std::cout << expected << std::endl;
    std::cout << value << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testfile okay." << std::endl;

  std::cout << " *** Testing the RegistrationModelXMLReader ***\n";

  RegistrationModelReaderPointer registrationModelReader = RegistrationModelReaderType::New();
  //   registrationModelReader->SetDebug(true);
  registrationModelReader->SetFilename("test.xml");
  registrationModelReader->GenerateOutputInformation();

  TubeSegmentModelPointer readTubeSegment = registrationModelReader->GetTubeSegment();
  PointSetPointer readPointSet = readTubeSegment->GetPointSet();
  PointType readStartPoint = readTubeSegment->GetStartPoint();

  if (startPoint != readStartPoint)
  {
    std::cout << "Start points do not match!" << std::endl;
    std::cout << "Written Point: " << startPoint << std::endl;
    std::cout << "Read Point:    " << readStartPoint << std::endl;
    return EXIT_FAILURE;
  }

  PointsIterator origPointsIter = pointsContainer->Begin();
  PointsIterator readPointsIter = readPointSet->GetPoints()->Begin();

  while (origPointsIter != pointsContainer->End())
  {
    PointSetPointType origPoint = origPointsIter.Value();
    PointSetPointType readPoint = readPointsIter.Value();

    if (origPoint != readPoint)
    {
      std::cout << "Points do not match." << std::endl;
      std::cout << "Orignal point: " << origPoint << std::endl;
      std::cout << "Read point:    " << readPoint << std::endl;
      return EXIT_FAILURE;
    }

    origPointsIter++;
    readPointsIter++;
  }

  PointSetPointer readConnectionPoints      = readTubeSegment->GetConnectionPoints();
  PointsIterator  readConnectionPointsIter  = readConnectionPoints->GetPoints()->Begin();
  PointSetPointType readConnectionPoint     = readConnectionPointsIter.Value();

  PointSetPointType origConnectionPoint     = tubeSegment->GetConnectionPoints()->GetPoints()->Begin().Value();

  if (readConnectionPoint != origConnectionPoint)
  {
    std::cout << "Connection points do not match." << std::endl;
    std::cout << "Orignal point: " << origConnectionPoint << std::endl;
    std::cout << "Read point:    " << readConnectionPoint << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST: Registrated Model
 ****************************************************************/
int testRegistratedModel()
{
  std::cout << " *** Testing the transfer of a model part via RegistratedModel ***\n";

  TubeSegmentModelPointer tubeSegment = generateTubeSegment();

  TransformOutputVectorType offset;
  offset.Fill(1);

  TransformPointer transform = TransformType::New();
  transform->SetOffset(offset);
  TransformParamtersType transformParameters = transform->GetParameters();

  RegistratedModelPointer registratedModel = RegistratedModelType::New();
  registratedModel->SetTransform(transform.GetPointer());
  registratedModel->SetTransformParameters(transformParameters);
  registratedModel->SetBaseModel(tubeSegment.GetPointer());

  PointType transformedStartPoint = transformPoint(transform, transformParameters, tubeSegment->GetStartPoint());
  PointType registratedStartPoint = registratedModel->GetStartPoint();
  if (transformedStartPoint != registratedStartPoint)
  {
    std::cout << "Start points do not match!" << std::endl;
    std::cout << "Transformed point: " << transformedStartPoint << std::endl;
    std::cout << "Point via RegistratedModel: " << registratedStartPoint << std::endl;
    return EXIT_FAILURE;
  }

  PointType transformedRotationPoint = transformPoint(transform, transformParameters, tubeSegment->GetRotationPoint());
  PointType registratedRotationPoint = registratedModel->GetRotationPoint();
  if (transformedRotationPoint != registratedRotationPoint)
  {
    std::cout << "Start points do not match!" << std::endl;
    std::cout << "Transformed point: " << transformedRotationPoint << std::endl;
    std::cout << "Point via RegistratedModel: " << registratedRotationPoint << std::endl;
    return EXIT_FAILURE;
  }

  PointSetPointer transformedConnectionPoints = transformPointSet(transform, transformParameters, tubeSegment->GetConnectionPoints());
  PointsIterator  transformedConPointsIter    = transformedConnectionPoints->GetPoints()->Begin();

  PointSetPointer registratedConnectionPoints = registratedModel->GetConnectionPoints();
  PointsIterator  registratedConPointsIter    = registratedConnectionPoints->GetPoints()->Begin();
  PointsIterator  lastRegistratedPoint        = registratedConnectionPoints->GetPoints()->End();

  while(registratedConPointsIter != lastRegistratedPoint)
  {
    PointSetPointType registratedPoint = registratedConPointsIter.Value();
    PointSetPointType transformedPoint = transformedConPointsIter.Value();

    if (registratedPoint != transformedPoint)
    {
      std::cout << "Connection points do not match!" << std::endl;
      std::cout << "Transformed point: " << transformedPoint << std::endl;
      std::cout << "Point via RegistratedModel: " << registratedPoint << std::endl;
      return EXIT_FAILURE;
    }

    ++transformedConPointsIter;
    ++registratedConPointsIter;
  }

  PointSetPointer transformedPointSet   = transformPointSet(transform, transformParameters, tubeSegment->GetPointSet());
  PointsIterator  transformedPointsIter = transformedPointSet->GetPoints()->Begin();

  PointSetPointer registratedPoints     = registratedModel->GetPointSet();
  PointsIterator  registratedPointsIter = registratedPoints->GetPoints()->Begin();
  PointsIterator  lastPoint             = registratedPoints->GetPoints()->End();

  while(registratedPointsIter != lastPoint)
  {
    PointSetPointType registratedPoint = registratedPointsIter.Value();
    PointSetPointType transformedPoint = transformedPointsIter.Value();

    if (registratedPoint != transformedPoint)
    {
      std::cout << "Points do not match!" << std::endl;
      std::cout << "Transformed point: " << transformedPoint << std::endl;
      std::cout << "Point via RegistratedModel: " << registratedPoint << std::endl;
      return EXIT_FAILURE;
    }

    ++registratedPointsIter;
    ++transformedPointsIter;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST: TreeToBinaryImageFilter
 ****************************************************************/
int testTreeToBinaryImageFilter()
{
  std::cout << " *** Testing the TreeToImageFilter ***\n";

  OutputTreePointer           outputTree    = OutputTreeType::New();
  OutputTreeContainerPointer  treeContainer = outputTree->GetTreeContainer();
  TransformPointer            transform     = TransformType::New();
  TubeSegmentModelPointer     tubeSegment   = generateTubeSegment();

  TransformOutputVectorType offset1;
  offset1.Fill(1);
  transform->SetOffset(offset1);
  RegistratedModelPointer model1 = RegistratedModelType::New();
  model1->SetBaseModel(tubeSegment.GetPointer());
  model1->SetTransform(transform.GetPointer());
  model1->SetTransformParameters(transform->GetParameters());
  treeContainer->SetRoot(model1.GetPointer());

  TransformOutputVectorType offset2;
  offset2.Fill(2);
  transform->SetOffset(offset2);
  RegistratedModelPointer model2 = RegistratedModelType::New();
  model2->SetBaseModel((RegistrationModelPointer)tubeSegment);
  model2->SetTransform((BaseTransformPointer)transform);
  model2->SetTransformParameters(transform->GetParameters());
  treeContainer->Add(static_cast<RegistrationModelPointer>(model2),
                     static_cast<RegistrationModelPointer>(model1));

  TreeToImageFilterPointer treeToImageFilter = TreeToImageFilterType::New();
  OutputImageType::SizeType size;
  size.Fill(100);
  OutputImageType::SpacingType spacing;
  spacing.Fill(1);
  treeToImageFilter->SetOutputImageSize(size);
  treeToImageFilter->SetOutputImageSpacing(spacing);
  treeToImageFilter->SetInput(outputTree);
  try
  {
    treeToImageFilter->Update();
  }
  catch(itk::ExceptionObject ex)
  {
    std::cout << "Exception occured" << std::endl;
    std::cout << ex << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}


/****************************************************************
 * TEST: ProfileGradientFinder
 ****************************************************************/
int testProfileGradientFinder()
{
  std::cout << " *** Testing the ProfileGradientFinder ***\n";

  OutputImagePointer image = OutputImageType::New();
  OutputImageType::SizeType size;
  size.Fill(100);
  image->SetRegions(size);
  image->Allocate();
  image->FillBuffer(0);
  OutputImageType::IndexType index;
  index.Fill(10);
  image->SetPixel(index, 255);

  ProfileGradientFinderType::VectorType startPoint;
  startPoint.Fill(0);
  ProfileGradientFinderType::VectorType endPoint;
  endPoint.Fill(50);

  itk::TimeProbe profileClock;

  ProfileGradientFinderPointer gradientFinder = ProfileGradientFinderType::New();
  gradientFinder->SetInput(image);
  gradientFinder->SetStartPoint(startPoint);
  gradientFinder->SetEndPoint(endPoint);

  profileClock.Start();
  gradientFinder->Update();
  profileClock.Stop();

  std::cout << "Counting profile took " << profileClock.GetMeanTime() << "s" << std::endl;

  ProfileDataType* profileData = gradientFinder->GetOutput();
  if(profileData == NULL)
  {
    std::cout << "Profile data is NULL!" << std::endl;
    return EXIT_FAILURE;
  }

  ProfileDataType::PointType bestPoint = profileData->GetPoint();
  const unsigned int imageDIMENSION = image->GetImageDimension();
  for(unsigned int dim = 0; dim < imageDIMENSION; dim++)
  {
    const double diff = bestPoint[dim] - 10;
    if (diff > 1.0 || diff < -1.0)
    {
      std::cout << "Point outisde of tolerance: " << diff << " at Point " << bestPoint << " dimension " << dim << std::endl;
      return EXIT_FAILURE;
    }
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST: RefinementModelProcessor
 ****************************************************************/
int testRefinementModelProcessor()
{
  std::cout << " *** Testing the RefinementModelProcessor ***\n";

  TubeSegmentModelGeneratorPointer tubeGenerator = TubeSegmentModelGeneratorType::New();

  tubeGenerator->SetNumberOfSlices(10);
  tubeGenerator->SetRadius(10);
  tubeGenerator->SetInnerRing(false);
  tubeGenerator->SetOuterRing(false);
  tubeGenerator->Update();
  TubeSegmentModelPointer tubeSegment = tubeGenerator->GetOutput();

  TransformPointer transform = TransformType::New();
  TransformOutputVectorType startPoint;
  startPoint.Fill(50);
  transform->SetOffset(startPoint);

  RegistratedModelPointer registratedModel = RegistratedModelType::New();
  registratedModel->SetBaseModel(static_cast<RegistrationModelPointer>(tubeSegment));
  registratedModel->SetTransform(static_cast<BaseTransformPointer>(transform));
  registratedModel->SetTransformParameters(transform->GetParameters());

  PointSetToImageFilterPointer pointSetToImageFilter = PointSetToImageFilterType::New();
  OutputImageType::SizeType size;
  size.Fill(100);
  pointSetToImageFilter->SetSize(size);
  pointSetToImageFilter->SetInput(registratedModel->GetPointSet());
  pointSetToImageFilter->Update();

  RefinementModelProcessorPointer refinementProcessor = RefinementModelProcessorType::New();
  refinementProcessor->SetInput(registratedModel);
  refinementProcessor->SetImage(pointSetToImageFilter->GetOutput());
  refinementProcessor->Update();

  RegistrationModelPointer  outputModel   = refinementProcessor->GetOutput();
  PointsContainerPointer    inputPoints   = tubeSegment->GetPointSet()->GetPoints();
  PointsContainerPointer    outputPoints  = outputModel->GetPointSet()->GetPoints();

  if(inputPoints->Size() != outputPoints->Size())
  {
    std::cout << "Output does not have the same number of points as input." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST: TubeSegmentRegistrator
 ****************************************************************/
int testTubeSegmentRegistrator()
{
  std::cout << " *** Testing the TubeSegmentRegistrator ***\n";

  TubeSegmentModelGeneratorPointer tubeGenerator = TubeSegmentModelGeneratorType::New();
  tubeGenerator->SetLength(10);
  tubeGenerator->SetNumberOfSlices(10);
  tubeGenerator->SetRadius(10);
  tubeGenerator->SetInnerRing(false);
  tubeGenerator->SetOuterRing(false);
  tubeGenerator->Update();
  TubeSegmentModelPointer tubeSegment = tubeGenerator->GetOutput();

  TubeSegmentModelType::PointType          tubeStartPoint       = tubeSegment->GetStartPoint();
  TubeSegmentModelType::PointSetPointType  tubeConnectionPoint  = tubeSegment->GetConnectionPoints()->GetPoints()->Begin().Value();

  RegistrationModelWriterPointer modelWriter = RegistrationModelWriterType::New();
  modelWriter->SetRegistrationModel(tubeSegment);
  modelWriter->SetFilename("test.xml");
  modelWriter->WriteFile();

  PointSetToImageFilterPointer pointSetToImageFilter = PointSetToImageFilterType::New();
  OutputImageType::SizeType size;
  size.Fill(100);
  pointSetToImageFilter->SetSize(size);
  pointSetToImageFilter->SetInput(tubeSegment->GetPointSet());
  pointSetToImageFilter->Update();

  OutputTreePointer outputTree = OutputTreeType::New();

  FilterContextPointer filterContext = FilterContextType::New();
  filterContext->SetInputImage(pointSetToImageFilter->GetOutput());
  filterContext->SetDryRun(true);
  filterContext->SetDryRun(true);
  filterContext->SetOutputTree(outputTree);
  filterContext->SetModelFilename("test.xml");

  StartPointDataPointer startPointData = StartPointDataType::New();
  StartPointDataType::PointType startPoint;
  startPoint.Fill(50);
  startPointData->SetStartPoint(startPoint);
  StartPointDataType::VectorType startDirection;
  startDirection.Fill(1.0);
  startPointData->SetStartDirection(startDirection);

  TubeSegmentModelRegistratorPointer tubeModelRegistrator = TubeSegmentModelRegistratorType::New();
  tubeModelRegistrator->SetFilterContext(filterContext);
  tubeModelRegistrator->SetStartPointData(startPointData);
  tubeModelRegistrator->Update();

  RegistrationModelPointer movedModel = filterContext->GetOutputTree()->GetTreeContainer()->GetRoot()->Get();
  RegistrationModelType::PointType          modelStartPoint       = movedModel->GetStartPoint();
  RegistrationModelType::PointSetPointType  modelConnectionPoint  = movedModel->GetConnectionPoints()->GetPoints()->Begin().Value();


  VectorType modelAxis;
  for(unsigned int dim = 0; dim < DIMENSION; dim++)
  {
    modelAxis[dim] = modelConnectionPoint[dim] - modelStartPoint[dim];
  }
  modelAxis.Normalize();

  if(modelAxis[0] == modelAxis[1] && modelAxis[1] == modelAxis[2])
  {
    std::cout << "Model StartPoint: " << modelStartPoint << std::endl;
    std::cout << "Model Axis: " << modelAxis << std::endl;

    std::cout << " *** [TEST PASSED] ***" << std::endl;
  }
  else
  {
    std::cout << "Model StartPoint: " << modelStartPoint << std::endl;
    std::cout << "Model Axis: " << modelAxis << std::endl;

    std::cout << " *** [TEST FAILED] ***" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/****************************************************************
 * TEST: RegistrationModel
 ****************************************************************/
int testRegistrationModelRadius()
{
  std::cout << " *** Testing the Radius calculation of the RegistrationModel ***\n";

  TubeSegmentModelGeneratorPointer tubeGenerator = TubeSegmentModelGeneratorType::New();
  tubeGenerator->SetLength(10);
  tubeGenerator->SetNumberOfSlices(10);
  tubeGenerator->SetRadius(10);
  tubeGenerator->SetInnerRing(false);
  tubeGenerator->SetOuterRing(false);
  tubeGenerator->Update();
  TubeSegmentModelPointer tubeSegment = tubeGenerator->GetOutput();

  ScalarType radius = tubeSegment->GetRadius(tubeSegment->GetConnectionPoints()->GetPoints()->Begin().Value());

  ScalarType diff = 10.0 - radius;
  if (diff > 1.0 || diff < -1.0)
  {
    std::cout << "Radius outside of tolerance: " << radius << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;
}

/*
 * WARNING: This test will crash on ITK > 2.6. ITK 2.4 and below is broken,
 *          when saving spatial objects to files.
 */
int testMeshReadWrite()
{
  std::cout << " *** Testing the Reading and Writing of meshes ***" << std::endl;

  std::cout << "--------------- WARNING ---------------" << std::endl;
  std::cout << "This test will crash on ITK > 2.6!" << std::endl;
  std::cout << "--------------- WARNING ---------------" << std::endl;

  TubeSegmentModelGeneratorPointer tubeGenerator = TubeSegmentModelGeneratorType::New();
  tubeGenerator->SetLength(10);
  tubeGenerator->SetNumberOfSlices(10);
  tubeGenerator->SetRadius(10);
  tubeGenerator->SetInnerRing(false);
  tubeGenerator->SetOuterRing(false);
  tubeGenerator->Update();
  TubeSegmentModelPointer tubeSegment = tubeGenerator->GetOutput();

  RegistrationModelWriterPointer modelWriter = RegistrationModelWriterType::New();
  modelWriter->SetRegistrationModel(tubeSegment);
  modelWriter->SetFilename("test.xml");
  modelWriter->WriteFile();

  RegistrationModelReaderPointer registrationModelReader = RegistrationModelReaderType::New();
  registrationModelReader->SetFilename("test.xml");
  registrationModelReader->GenerateOutputInformation();

  TubeSegmentModelPointer readTubeSegment = registrationModelReader->GetTubeSegment();

  MeshPointer mesh      = tubeSegment->GetMesh();
  MeshPointer readMesh  = readTubeSegment->GetMesh();

  if(mesh->GetPoints()->Size() != readMesh->GetPoints()->Size() &&
      mesh->GetPointData()->Size() != readMesh->GetPointData()->Size() &&
      mesh->GetCells()->Size() != readMesh->GetCells()->Size())
  {
    std::cout << "Meshes do not match." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << " *** [TEST PASSED] ***\n";
  return EXIT_SUCCESS;

}

int itkImageToTreeFilterTest(int /*i*/, char** /*argv[]*/)
{
  ResultListType resultList;
  int failedCount = 0;
  int testCount;
  float failRatio;

  resultList.push_back(testRegistrationModelXMLWriter());
  resultList.push_back(testRegistratedModel());
  resultList.push_back(testTreeToBinaryImageFilter());
  resultList.push_back(testProfileGradientFinder());
  resultList.push_back(testRefinementModelProcessor());
  resultList.push_back(testRegistrationModelRadius());
  resultList.push_back(testTubeSegmentRegistrator());
  resultList.push_back(testMeshReadWrite());

  std::cout << " *** [ALL TESTS DONE] ***\n";

  testCount = resultList.size();
  bool allSuccess = true;

  while (resultList.size() > 0)
  {
    int value = resultList.front();
    if(value == EXIT_FAILURE) 
    { 
      failedCount++; 
      allSuccess = false;
    }
    resultList.pop_front();
  }

  failRatio = 100 * (float) failedCount / (float) testCount;

  std::cout << "Result: " << failedCount << "/" << testCount << " Tests failed (" << failRatio << " %)" << std::endl;

  if(allSuccess)
  {
    return EXIT_SUCCESS;
  }
  else 
  {
    return EXIT_FAILURE;
  }
}
