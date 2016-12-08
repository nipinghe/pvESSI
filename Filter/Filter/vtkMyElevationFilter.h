/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMyElevationFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMyElevationFilter - generate scalars along a specified direction
// .SECTION Description
// vtkMyElevationFilter is a filter to generate scalar values from a
// dataset.  The scalar values lie within a user specified range, and
// are generated by computing a projection of each dataset point onto
// a line. The line can be oriented arbitrarily. A typical example is
// to generate scalars based on elevation or height above a plane.

#ifndef vtkMyElevationFilter_h
#define vtkMyElevationFilter_h

#include <stdlib.h>
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkQuadratureSchemeDefinition.h"
#include "vtkQuadraturePointInterpolator.h"
#include "vtkQuadraturePointsGenerator.h"
#include "vtkQuadratureSchemeDictionaryGenerator.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkIdTypeArray.h"
#include "vtkInformationVector.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkLookupTable.h"
#include "vtkDelaunay3D.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include "vtkExecutive.h"
#include <sstream>
#include "hdf5.h"
#include "vtkProbeFilter.h"
#include <vtkDelaunay3D.h>
#include "vtkSelectionNode.h"
#include "vtkExtractSelection.h"
#include "vtkSelection.h"
#include "vtkElevationFilter.h"

class VTK_EXPORT vtkMyElevationFilter : public vtkElevationFilter
{
public:
  static vtkMyElevationFilter* New();
  static vtkInformationStringKey* FILE_NAME();
  vtkTypeMacro(vtkMyElevationFilter, vtkElevationFilter);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int RequestInformation( vtkInformation *, vtkInformationVector **, vtkInformationVector* );

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of the .abc file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkMyElevationFilter();
  ~vtkMyElevationFilter();

private:
  vtkMyElevationFilter(const vtkMyElevationFilter&);  // Not implemented.
  void operator=(const vtkMyElevationFilter&);  // Not implemented.

    vtkSmartPointer<vtkUnstructuredGrid> UGrid_Node_Mesh;                 // Contains the mesh of all domains


    char* FileName;          // Holds the filename
};

#endif
