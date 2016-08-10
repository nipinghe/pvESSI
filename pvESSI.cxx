#include "pvESSI.h"
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

// LINK_LIBRARIES(hdf5_cpp hdf5 )

/************************************************************************************************************************************************/
// cmake .. -DParaView_DIR=~/Softwares/Paraview/Paraview-Build/ -DGHOST_BUILD_CDAWEB=OFF
// cmake .. -DParaView_DIR="/home/sumeet/Softwares/ParaView-v4.4.0" -DGHOST_BUILD_CDAWEB=OFF
/************************************************************************************************************************************************/


vtkStandardNewMacro(pvESSI);

pvESSI::pvESSI(){ 

	this->FileName = NULL;
	this->SetNumberOfInputPorts(0);
	this->SetNumberOfOutputPorts(1);
	UGrid_Gauss_Mesh          = vtkSmartPointer<vtkUnstructuredGrid>::New();
	UGrid_Node_Mesh           = vtkSmartPointer<vtkUnstructuredGrid>::New();
	UGrid_Current_Node_Mesh   = vtkSmartPointer<vtkUnstructuredGrid>::New();
	UGrid_Current_Gauss_Mesh  = vtkSmartPointer<vtkUnstructuredGrid>::New();
	this->set_VTK_To_ESSI_Elements_Connectivity();
	this->Build_Meta_Array_Map();
	Build_Inverse_Matrices();
	Build_Gauss_To_Node_Interpolation_Map();
}

/*****************************************************************************
* This method responds to the request made by vtk Pipeleine. 
* This method is invoked when the time stamp is changed from paraview VCR.
*****************************************************************************/

int pvESSI::RequestData(vtkInformation *vtkNotUsed(request),vtkInformationVector **vtkNotUsed(inputVector),	vtkInformationVector *outputVector){
 
	// get the info object
	vtkInformation *Node_Mesh = outputVector->GetInformationObject(0);
	// vtkInformation *Gauss_Mesh = outputVector->GetInformationObject(1);
	// outInfo->Print(std::cout);

	if (!Whether_Node_Mesh_Build){
		this->Get_Node_Mesh(UGrid_Node_Mesh);
		UGrid_Current_Node_Mesh->ShallowCopy(UGrid_Node_Mesh);
	} 

	// if (!Whether_Gauss_Mesh_Build ){ 
	// 	this->Get_Gauss_Mesh(UGrid_Gauss_Mesh);
	// 	UGrid_Current_Gauss_Mesh->ShallowCopy(UGrid_Gauss_Mesh);
	// } 

	// int extent[6] = {0,-1,0,-1,0,-1};
	// outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);

  	this->Node_Mesh_Current_Time = Time_Map.find( Node_Mesh->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))->second;
  	// this->Gauss_Mesh_Current_Time = Time_Map.find( Gauss_Mesh->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))->second;

  	// int Clength = outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  	// double* Csteps = outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  	// if(Gauss_Mesh_Current_Time > Number_Of_Time_Steps){
  	// 	Gauss_Mesh_Current_Time =0;
  	// }


 	// std::cout<< "Node_Mesh_Current_Time " << Node_Mesh_Current_Time<< std::endl;
 	// std::cout<< "Gauss_Mesh_Current_Time " << Gauss_Mesh_Current_Time<< std::endl;

 	/////////////////////////////////  Printing For Debugging ////////////////////////////////
 	// cout << "Number of Nodes "   << " " << Number_of_Nodes << endl;
	// cout << "Pseudo_Number_of_Nodes "  << " " << Pseudo_Number_of_Nodes << endl;
	// cout << "Number of Gauss Points " << Number_of_Gauss_Nodes << endl;
 	// cout << "Number of Elements "   << " " << Number_of_Elements << endl;
	// cout << "Pseudo_Number_of_Elements"  << " " << Pseudo_Number_of_Elements << endl;
	/////////////////////////////////////////////////////////////////////////////////////////


 	// cout<< "Gauss_Mesh_Current_Time " << Gauss_Mesh_Current_Time<< endl;
	// cout<< "Clength " << "  " << vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP() << endl;
	// for (int i =0 ; i< Clength ; i++){
	// 	cout << Csteps[i] << "  " << endl;
	// }

    // cout << Node_Mesh_Current_Time/Time[0] << endl;

	// if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
	//          std::cout << "Update Time" << endl;
	//     if (!outInfo->Has(vtkDataObject::DATA_TIME_STEP()))
	    		// std::cout << "DATA_TIME_STEP" << endl;

	// return outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP())
	// int piece, numPieces;
	// piece = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
	// numPieces = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());

	/////////////////////////////// Read in Paralll //////////////////////////////
	// outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);
	///////////////////////////////////////////////////////////////////////////////

	// cout << "Node Atributes  Started " <<endl;
	Build_Node_Attributes(UGrid_Current_Node_Mesh, this->Node_Mesh_Current_Time );
	Build_Stress_Field_At_Nodes(UGrid_Current_Node_Mesh, this->Node_Mesh_Current_Time);
	
	// Build_Gauss_Attributes(UGrid_Current_Gauss_Mesh, this->Gauss_Mesh_Current_Time );

	// get the ouptut pointer to paraview 
	vtkUnstructuredGrid *Output_Node_Mesh = vtkUnstructuredGrid::SafeDownCast(Node_Mesh->Get(vtkDataObject::DATA_OBJECT()));
	// vtkUnstructuredGrid *Output_Gauss_Mesh = vtkUnstructuredGrid::SafeDownCast(Gauss_Mesh->Get(vtkDataObject::DATA_OBJECT()));

	Output_Node_Mesh->ShallowCopy(UGrid_Current_Node_Mesh);
	// Output_Gauss_Mesh->ShallowCopy(UGrid_Current_Gauss_Mesh);

	return 1;
}

// ****************************************************************************
// * This method is called only once for information about time stamp and extent. 
// * This is the method which is called firt after the default constructor is 
// * initialized
// ****************************************************************************

int pvESSI::RequestInformation( vtkInformation *request, vtkInformationVector **vtkNotUsed(inVec), vtkInformationVector* outVec){

	this->initialize();

	vtkInformation* Node_Mesh = outVec->GetInformationObject(0);
	// vtkInformation* Gauss_Mesh = outVec->GetInformationObject(1);

	////////////////////////////////////////////////////// Reading General Outside Data /////////////////////////////////////////////////////////////////////////////

	H5Dread(id_Number_of_Time_Steps, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&Number_Of_Time_Steps);
	H5Dclose(id_Number_of_Time_Steps);

	H5Dread(id_Number_of_Elements, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&Number_of_Elements);
	H5Dclose(id_Number_of_Elements);

	H5Dread(id_Number_of_Nodes, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&Number_of_Nodes);
	H5Dclose(id_Number_of_Nodes);

	DataSpace = H5Dget_space(id_Index_to_Coordinates);
	H5Sget_simple_extent_dims(DataSpace, dims1_out, NULL);
	this->Pseudo_Number_of_Nodes = dims1_out[0];
	H5Sclose(DataSpace);

	DataSpace = H5Dget_space(id_Index_to_Connectivity);
	H5Sget_simple_extent_dims(DataSpace, dims1_out, NULL);
	this->Pseudo_Number_of_Elements = dims1_out[0];
	H5Sclose(DataSpace);

	if(id_Whether_Maps_Build < 0){

		cout << "<<<<pvESSI>>>> Whether_Maps_Build Dataset Not Build, So lets go and build it first \n " << endl;

		dims1_out[0]=1; DataSpace = H5Screate_simple(1, dims1_out, NULL);
		id_Whether_Maps_Build = H5Dcreate(id_File,"Whether_Maps_Build",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);

		this->Build_Map_Status = -1; 

		H5Sclose(DataSpace); 

		cout << "<<<<pvESSI>>>> Maps are now built \n " << endl;
	}
	else{
		H5Dread(id_Whether_Maps_Build, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,&Build_Map_Status);
	}

   	if(this->Build_Map_Status != 1){
   		
   		cout << "<<<<pvESSI>>>> Maps Not Built So Lets Go and Create the Maps \n" << endl;
   		Build_Maps();

		count1[0]   =1;
		dims1_out[0]=1;
		index_i=0;
		this->Build_Map_Status =1;

        HDF5_Write_INT_Array_Data(id_Whether_Maps_Build,
			                        1,
			                       dims1_out,
			                       &index_i,
			                       NULL,
			                       count1,
			                       NULL,
			                       &Build_Map_Status); // Build_Map_Status

   	}

   	cout << "<<<<pvESSI>>>> Maps are build HURRAY!!! \n" << endl;

	this->Time = new double[Number_Of_Time_Steps]; 
	H5Dread(id_time, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,Time);  
	H5Dclose(id_time);

	Build_Time_Map(); 

	double Time_range[2]={Time[0],Time[Number_Of_Time_Steps-1]};

	Node_Mesh->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),Time, this->Number_Of_Time_Steps);
	Node_Mesh->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),Time_range,2);

	// Gauss_Mesh->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),Time, this->Number_Of_Time_Steps);
	// Gauss_Mesh->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),Time_range,2);

	// outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),extent,6);
	// outInfo->Set(vtkAlgorithm::CAN_PRODUCE_SUB_EXTENT(),1);

	// /////////////////////////////////////////////// Assign Key /////////////////////////////////////////////////////
	// // vtkSmartPointer<vtkInformationQuadratureSchemeDefinitionVectorKey> Quadrature_Definition_Vector = vtkSmartPointer<vtkInformationQuadratureSchemeDefinitionVectorKey> ::New();

	// // vtkSmartPointer<vtkQuadratureSchemeDefinition> Quadrature_Definition = vtkSmartPointer<vtkQuadratureSchemeDefinition>::New();
	// // Quadrature_Definition->Initialize(VTK_HEXAHEDRON,8,8,W_QQ_64_A);

	return 1;
}


/*****************************************************************************
* This method creates and process the request from vtk Pipeleine
*****************************************************************************/

int pvESSI::ProcessRequest(vtkInformation  *request_type, vtkInformationVector  **inInfo, vtkInformationVector *outInfo){

	//////////////////////////////////// Printing for Debugging /////////////////////////////////////////
	// cout << "-----------------------------------------------------------------------------------\n";
	// vtkIndent indent;
 	// request_type->PrintSelf(std::cout, indent);
 	////////////////////////////////////////////////////////////////////////////////////////////////////

  return this->Superclass::ProcessRequest(request_type, inInfo, outInfo);
}

/*****************************************************************************
* This method prints about reader plugin i.e itself
*****************************************************************************/
void pvESSI::PrintSelf(ostream& os, vtkIndent indent){

	this->Superclass::PrintSelf(os,indent);
	os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
	return;
}

/*****************************************************************************
* This method creates a map for the connectivity order from ESSI connectivity
* to vtk elements connectivity. The connectivity order is stored in\
* ESSI_to_VTK_Element map whose key is the number of connectivity nodes. 
*
* !!!I think a better key is needed so that it can be robust.
*****************************************************************************/

void pvESSI::set_VTK_To_ESSI_Elements_Connectivity(){

	std::vector<int> connectivity_vector;

	ESSI_to_VTK_Element[1] = VTK_VERTEX;		ESSI_to_VTK_Element[2]  = VTK_LINE;					ESSI_to_VTK_Element[4] = VTK_QUAD;
	ESSI_to_VTK_Element[8] = VTK_HEXAHEDRON;	ESSI_to_VTK_Element[20] = VTK_QUADRATIC_HEXAHEDRON;	ESSI_to_VTK_Element[27] = VTK_TRIQUADRATIC_HEXAHEDRON;

	int Node[1] = {0};						connectivity_vector.assign(Node,Node+1);				ESSI_to_VTK_Connectivity[1] = connectivity_vector;	connectivity_vector.clear();
	int Line[2] = {0,1}; 					connectivity_vector.assign(Line,Line+2); 				ESSI_to_VTK_Connectivity[2] = connectivity_vector;	connectivity_vector.clear();
	int Quadrangle[4] = {0,1,2,3}; 			connectivity_vector.assign(Quadrangle,Quadrangle+4); 	ESSI_to_VTK_Connectivity[4] = connectivity_vector;	connectivity_vector.clear();
	int Hexahedron[8] = {4,5,6,7,0,1,2,3}; 	connectivity_vector.assign(Hexahedron,Hexahedron+8);	ESSI_to_VTK_Connectivity[8] = connectivity_vector;	connectivity_vector.clear();
	int Qudratic_hexahedron[20] = {6,5,4,7,2,1,0,3,13,12,15,14,9,8,11,10,18,17,16,19}; 	connectivity_vector.assign(Qudratic_hexahedron,Qudratic_hexahedron+20);	ESSI_to_VTK_Connectivity[20]= connectivity_vector;	connectivity_vector.clear();
	int TriQudratic_hexahedron[27] = {6,5,4,7,2,1,0,3,13,12,15,14,9,8,11,10,18,17,16,19,23,21,22,24,26,25,20}; 	connectivity_vector.assign(TriQudratic_hexahedron,TriQudratic_hexahedron+27);	ESSI_to_VTK_Connectivity[27]= connectivity_vector;	connectivity_vector.clear();
	return;
}

/*****************************************************************************
* This Method builds node attributes for the current time and pushes it to 
* the <vtkUnstructuredGrid> input vtkobject.
*****************************************************************************/

void pvESSI::Build_Node_Attributes(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Current_Time){


	//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Node_Map[Number_of_Nodes];
	H5Dread(id_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Map); 

	int Element_Map[Number_of_Elements];
	H5Dread(id_Element_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Map); 

	////////////////////////////////////////////////////// Reading Node Attributes /////////////////////////////////////////////////////////////////////////////

	int Node_Index_to_Generalized_Displacements[Pseudo_Number_of_Nodes];
	H5Dread(id_Index_to_Generalized_Displacements, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Index_to_Generalized_Displacements); 

	double Element_Material_Tags[Pseudo_Number_of_Nodes];
	H5Dread(id_Material_tags, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Material_Tags); 

	///////////////////////////////////////////  Output Dataset for a particular time /////////////////////////////////////////////////////////////////////////////	

	DataSpace = H5Dget_space(id_Generalized_Displacements);
	H5Sget_simple_extent_dims(DataSpace, dims2_out, NULL);	
	float Node_Generalized_Displacements[dims2_out[0]];
	offset2[0]=0; 					  count2[0] = dims2_out[0];		dims1_out[0]=dims2_out[0];
	offset2[1]=Current_Time; 		  count2[1] = 1;				MemSpace = H5Screate_simple(1,dims1_out,NULL);
	H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset2,NULL,count2,NULL);
	H5Dread(id_Generalized_Displacements, H5T_NATIVE_FLOAT, MemSpace, DataSpace, H5P_DEFAULT, Node_Generalized_Displacements); 
	H5Sclose(MemSpace);
	H5Sclose(DataSpace); 

 	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 	/////////////////////////////////////////////////////////////// DataSets Visulization at Nodes //////////////////////////////////////////////////////////////////////////////////////
 	
 	Generalized_Displacements = vtkSmartPointer<vtkFloatArray>::New(); 
	this->Set_Meta_Array (Meta_Array_Map["Generalized_Displacements"]);

	Material_Tag = vtkSmartPointer<vtkIntArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Material_Tag"]);

	Node_Tag = vtkSmartPointer<vtkIntArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Node_Tag"]);

	Element_Tag = vtkSmartPointer<vtkIntArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Element_Tag"]);

	for (int i = 0; i < Number_of_Nodes; i++){

				node_no = Node_Map[i]; Node_Tag -> InsertValue(i,node_no);

				float tuple[3]={
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[node_no]  ],
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[node_no]+1],
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[node_no]+2]
				};	

				Generalized_Displacements->InsertTypedTuple(i,tuple);


				// ************************************************************** Displacement, Acceleration and Velocity -> Calculation Formulae *********************************************************

				// Displacement(t) = D_t;
				// Acceleration(t) = (1/del.t)^2*{ D_(t+del.t) - 2*D_t + D_(t-del.t)};
				// Acceleration(t) = 1/2/del.t*{ D_(t+del.t) - D_(t-del.t)};

				/***************************************************************** Add Acceleration and Velocity Arrays Here ****************************************/
					
				/****************************************************************************************************************************************************/

	}

	Node_Mesh->GetPointData()->AddArray(Generalized_Displacements);
	Node_Mesh->GetPointData()->AddArray(Node_Tag);

	// /////////////////////////////////////////////////////////////////////// DataSet Visualization in Cell   ///////////////////////////////////////////////////////////////
 	int Element_No; 
	for (int i = 0; i < Number_of_Elements; i++){

		Element_No = Element_Map[i]; 
		Element_Tag -> InsertValue(i,Element_No);
		Material_Tag -> InsertValue(i,Element_Material_Tags[Element_No]);

	}

	Node_Mesh->GetCellData()->AddArray(Element_Tag);
	Node_Mesh->GetCellData()->AddArray(Material_Tag);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	return;
}

/*****************************************************************************
* This Method builds gauss attributes for the current time and pushes it to 
* the <vtkUnstructuredGrid> input vtkobject.
*****************************************************************************/
void pvESSI::Build_Gauss_Attributes(vtkSmartPointer<vtkUnstructuredGrid> Gauss_Mesh, int Current_Time){

	// this->Build_ProbeFilter_Gauss_Mesh(Gauss_Mesh, 1);

	//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Element_Map[Number_of_Elements];
	H5Dread(id_Element_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Map); 

	////////////////////////////////////////////////////// Reading Element  Attributes /////////////////////////////////////////////////////////////////////////////

	int Element_Index_to_Outputs[Pseudo_Number_of_Elements];
	H5Dread(id_Index_to_Outputs, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Index_to_Outputs);

	int Element_Number_of_Gauss_Points[Pseudo_Number_of_Elements];
	H5Dread(id_Number_of_Gauss_Points, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Gauss_Points);

	int Element_Number_of_Output_Fields[Pseudo_Number_of_Elements];
	H5Dread(id_Number_of_Output_Fields, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Output_Fields);


	///////////////////////////////////////////  Output Dataset for a particular time /////////////////////////////////////////////////////////////////////////////	

	DataSpace = H5Dget_space(id_Outputs);
	H5Sget_simple_extent_dims(DataSpace, dims2_out, NULL);	
	float Element_Outputs[dims2_out[0]];
	offset2[0]=0; 					   count2[0] = dims2_out[0];		dims1_out[0]=dims2_out[0];
	offset2[1]=Current_Time; 		   count2[1] = 1;					MemSpace = H5Screate_simple(1,dims1_out,NULL);
	H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset2,NULL,count2,NULL);
	H5Dread(id_Outputs, H5T_NATIVE_FLOAT, MemSpace, DataSpace, H5P_DEFAULT, Element_Outputs); 
	H5Sclose(MemSpace); 
	H5Sclose(DataSpace); 

	///////////////////////////////////////////////////////////////////////////////// Building up the elements //////////////////////////////////////////////////////

	Elastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Elastic_Strain"]);

	Plastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Plastic_Strain"]);

	Stress = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Stress"]);


	//////////////////////////////////// Usefull information about tensor components order ///////////////////////////////////////////////////////////////////////////
	// > Symmetric Tensor the order is 0 1 2 1 3 4 2 4 5 so like in a matrix
	// > 0 1 2
	// > 1 3 4
	// > 2 4 6

	// > Assymetriic Tensor tensor the order is  0 1 2 3 4 5 6 7 8
	// > 0 1 2
	// > 3 4 5
	// > 6 7 8
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	int Index_of_Gauss_Nodes =0, element_no, No_of_Element_Gauss_Nodes,Output_Index, gauss_number=0;

	for (int i = 0; i < Number_of_Elements; i++){

		element_no = Element_Map[i];

		No_of_Element_Gauss_Nodes = Element_Number_of_Gauss_Points[element_no];
		Output_Index = Element_Index_to_Outputs[element_no];

		for(int j=0; j< No_of_Element_Gauss_Nodes ; j++){

			float El_Strain_Tuple[6] ={
				Element_Outputs[Output_Index  ],
				Element_Outputs[Output_Index+3],
				Element_Outputs[Output_Index+4],
				Element_Outputs[Output_Index+1],
				Element_Outputs[Output_Index+5],
				Element_Outputs[Output_Index+2]
			};

			Output_Index = Output_Index+6;

			float Pl_Strain_Tuple[6] ={
				Element_Outputs[Output_Index  ],
				Element_Outputs[Output_Index+3],
				Element_Outputs[Output_Index+4],
				Element_Outputs[Output_Index+1],
				Element_Outputs[Output_Index+5],
				Element_Outputs[Output_Index+2]
			};

			Output_Index = Output_Index+6;

			float Stress_Tuple[6] ={
				Element_Outputs[Output_Index  ],
				Element_Outputs[Output_Index+3],
				Element_Outputs[Output_Index+4],
				Element_Outputs[Output_Index+1],
				Element_Outputs[Output_Index+5],
				Element_Outputs[Output_Index+2]
			};

			Output_Index = Output_Index+6;

			Elastic_Strain->InsertTypedTuple (Index_of_Gauss_Nodes,El_Strain_Tuple);
			Plastic_Strain->InsertTypedTuple (Index_of_Gauss_Nodes,Pl_Strain_Tuple);
			Stress->InsertTypedTuple (Index_of_Gauss_Nodes,Stress_Tuple);
			
			Index_of_Gauss_Nodes+=1;

		}
	}

	Gauss_Mesh->GetPointData()->AddArray(Stress);
  	Gauss_Mesh->GetPointData()->AddArray(Plastic_Strain);
  	Gauss_Mesh->GetPointData()->AddArray(Elastic_Strain);

	return;
}

/*****************************************************************************
* This Method probes node mesh variables at gauss mesh 
* By Default [prob_type =0], probes only generalized displacement
* But the user can choose an option [prob_type = 1] to probe all variables
*****************************************************************************/

void pvESSI::Build_ProbeFilter_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> Probe_Input, int probe_type){

	vtkSmartPointer<vtkUnstructuredGrid> Probe_Source = vtkSmartPointer<vtkUnstructuredGrid>::New();
	Probe_Source->ShallowCopy(this->UGrid_Node_Mesh);

	if (probe_type){
		Build_Node_Attributes(Probe_Source, this->Gauss_Mesh_Current_Time);
	}
	else{

		herr_t status;
		hid_t File, DataSet, DataSpace, MemSpace; 
		hsize_t  dims_out[2], offset[2], count[2], col_dims[1];

		//////////////////////////////////////////////////////// Reading Hdf5 File ///////////////////////////////////////////////////////////////////////////////////////

		File = H5Fopen(this->FileName, H5F_ACC_RDONLY, H5P_DEFAULT);

		//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////
		
		int Node_Map[Number_of_Nodes];
		H5Dread(id_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Map); 

		////////////////////////////////////////////////////// Reading Node Attributes /////////////////////////////////////////////////////////////////////////////
		
		int Node_Index_to_Generalized_Displacements[Pseudo_Number_of_Nodes];
		H5Dread(id_Index_to_Generalized_Displacements, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Index_to_Generalized_Displacements); 

		///////////////////////////////////////////  Output Dataset for a particular time /////////////////////////////////////////////////////////////////////////////	

		DataSpace = H5Dget_space(id_Generalized_Displacements);
		H5Sget_simple_extent_dims(DataSpace, dims2_out, NULL);	
		float Node_Generalized_Displacements[dims2_out[0]];
		offset2[0]=0; 					  				  count2[0] = dims2_out[0];		dims1_out[0]=dims2_out[0];
		offset2[1]=this->Gauss_Mesh_Current_Time; 		  count2[1] = 1;				MemSpace = H5Screate_simple(1,dims1_out,NULL);
		H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset2,NULL,count2,NULL);
		H5Dread(id_Generalized_Displacements, H5T_NATIVE_FLOAT, MemSpace, DataSpace, H5P_DEFAULT, Node_Generalized_Displacements); 
		H5Sclose(MemSpace);
		H5Sclose(DataSpace); 

	 	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Generalized_Displacements = vtkSmartPointer<vtkFloatArray>::New(); 
		this->Set_Meta_Array (Meta_Array_Map["Generalized_Displacements"]);
 	
	 	/////////////////////////////////////////////////////////////// DataSets Visulization at Nodes //////////////////////////////////////////////////////////////////////////////////////
	 	int Node_No; 
		for (int i = 0; i < Number_of_Nodes; i++){

				Node_No = Node_Map[i]; 
				float tuple[3]={
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[Node_No]],
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[Node_No]+1],
					Node_Generalized_Displacements[Node_Index_to_Generalized_Displacements[Node_No]+2]
				};	

				Generalized_Displacements->InsertTypedTuple(i,tuple);


		}

		Probe_Source->GetPointData()->AddArray(Generalized_Displacements);

	}

	/************* Initializing Probe filter ******************************************/

	vtkSmartPointer<vtkProbeFilter> ProbeFilter = vtkSmartPointer<vtkProbeFilter>::New();
	ProbeFilter->SetSourceData(Probe_Source);
	ProbeFilter->SetInputData(Probe_Input);
	ProbeFilter->Update();

	Probe_Input->ShallowCopy( ProbeFilter->GetOutput());

	////////////////////////////////////// For Debugging ////////////////////////////////
	UGrid_Gauss_Mesh->GetPointData()->RemoveArray("Material_Tag");
	// UGrid_Gauss_Mesh->GetPointData()->RemoveArray("Node_No");
	UGrid_Gauss_Mesh->GetPointData()->RemoveArray("Element_No");
	/////////////////////////////////////////////////////////////////////////////////////

  	return;

}


/*****************************************************************************
* This function creates a Node Mesh i.e a mesh from the given node data 
* and element data from hdf5 file. The function stores the mesh in the 
* given input <vtkUnstructuredGrid> input object.
*****************************************************************************/

void pvESSI::Get_Node_Mesh(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh){


	//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Node_Map[Number_of_Nodes];
	H5Dread(id_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Map); 

	int Inverse_Node_Map[Pseudo_Number_of_Nodes];
	H5Dread(id_Inverse_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Inverse_Node_Map); 

	int Element_Map[Number_of_Elements];
	H5Dread(id_Element_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Map); 

	////////////////////////////////////////////////////// Reading Node Attributes /////////////////////////////////////////////////////////////////////////////

	int Node_Index_to_Coordinates[Pseudo_Number_of_Nodes];
	H5Dread(id_Index_to_Coordinates, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Index_to_Coordinates); 

	double Node_Coordinates[Number_of_Nodes*3];
	H5Dread(id_Coordinates, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Coordinates); 

	//////////////////////////////////////////////////// Reading Element Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Element_Index_to_Connectivity[Pseudo_Number_of_Elements];
	H5Dread(id_Index_to_Connectivity, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Index_to_Connectivity); 

	int Element_Class_Tags[Pseudo_Number_of_Elements];
	H5Dread(id_Class_Tags, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Class_Tags);

	int Element_Number_of_Nodes[Pseudo_Number_of_Elements];
	H5Dread(id_Element_Number_of_Nodes, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Nodes);

	DataSpace = H5Dget_space(id_Connectivity);
	H5Sget_simple_extent_dims(DataSpace, dims1_out, NULL);	
	int Element_Connectivity[dims1_out[0]];
	H5Sclose(DataSpace); 
	H5Dread(id_Connectivity, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Connectivity); 

	/////////////////////////////////////////////////// Building Nodes ///////////////////////////////////////////////////////////////////////////////////////////

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

	points->SetNumberOfPoints(this->Number_of_Nodes);
	Node_Mesh->Allocate(this->Number_of_Elements);

	for (int i = 0; i < Number_of_Nodes; i++){
		node_no = Node_Map[i];
		points->InsertPoint(i,  Node_Coordinates[Node_Index_to_Coordinates[node_no]],
								Node_Coordinates[Node_Index_to_Coordinates[node_no]+1],
								Node_Coordinates[Node_Index_to_Coordinates[node_no]+2]
		);
	}

	Node_Mesh->SetPoints(points);

	///////////////////////////////////////////////////////////////////////////// Building up the elements //////////////////////////////////////////////////////

	int connectivity_index,No_of_Element_Nodes,Cell_Type;

	for (int i = 0; i < Number_of_Elements; i++){

		element_no = Element_Map[i];

		connectivity_index = Element_Index_to_Connectivity[element_no];
		No_of_Element_Nodes = Element_Number_of_Nodes[element_no];

		vtkIdType Vertices[No_of_Element_Nodes];
		Cell_Type = ESSI_to_VTK_Element.find(No_of_Element_Nodes)->second;
		std::vector<int> Nodes_Connectivity_Order = ESSI_to_VTK_Connectivity.find(No_of_Element_Nodes)->second;

		for(int j=0; j<No_of_Element_Nodes ; j++){
			Vertices[j] = Inverse_Node_Map[Element_Connectivity[connectivity_index+Nodes_Connectivity_Order[j]]];
		}

		Node_Mesh->InsertNextCell(Cell_Type, No_of_Element_Nodes, Vertices);

	}

	Whether_Node_Mesh_Build=1;

	return;
	
}

/*****************************************************************************
* This Function builds a gauss mesh from the given gauss mesh coordinates  
* Uses Delaunay3D filter to generate the mesh 
*****************************************************************************/

void pvESSI::Get_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> Gauss_Mesh){

	//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Element_Map[Number_of_Elements];
	H5Dread(id_Element_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Map); 


	/////////////////////////////////////////////////////////////////////// Reading Element Data ///////////////////////////////////////////////////////////////////////////////////////////

	int Element_Index_to_Gauss_Point_Coordinates[Pseudo_Number_of_Elements];
	H5Dread(id_Index_to_Gauss_Point_Coordinates, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Index_to_Gauss_Point_Coordinates);

	int Element_Number_of_Gauss_Points[Pseudo_Number_of_Elements];
	H5Dread(id_Number_of_Gauss_Points, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Gauss_Points);

	DataSpace = H5Dget_space(id_Gauss_Point_Coordinates);
	H5Sget_simple_extent_dims(DataSpace, dims1_out, NULL);	
	float Element_Gauss_Point_Coordinates[dims1_out[0]];
	H5Dread(id_Gauss_Point_Coordinates, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Gauss_Point_Coordinates); 
	status=H5Sclose(DataSpace); 

	////////////////////////////////////////////////////////////////////// Building up the elements //////////////////////////////////////////////////////

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->SetNumberOfPoints(Number_of_Gauss_Nodes);
	int No_of_Element_Gauss_Nodes; vtkIdType onevertex;

	this->Number_of_Gauss_Nodes =0;

	for (int i=0; i < Number_of_Elements; i++){
		element_no = Element_Map[i];
		No_of_Element_Gauss_Nodes = Element_Number_of_Gauss_Points[element_no];
		for(int j=0; j<No_of_Element_Gauss_Nodes ; j++){
			points->InsertPoint(Number_of_Gauss_Nodes, Element_Gauss_Point_Coordinates[Element_Index_to_Gauss_Point_Coordinates[element_no]+3*j],Element_Gauss_Point_Coordinates[Element_Index_to_Gauss_Point_Coordinates[element_no]+3*j+1],Element_Gauss_Point_Coordinates[Element_Index_to_Gauss_Point_Coordinates[element_no]+3*j+2]);
			/// Adding 1D point to mesh 
			onevertex = Number_of_Gauss_Nodes;
			Gauss_Mesh->InsertNextCell(VTK_VERTEX, 1, &onevertex);

			/// updating number_of_gauss_nodes
			Number_of_Gauss_Nodes = Number_of_Gauss_Nodes+1;
		}

	}

	Gauss_Mesh->SetPoints(points);

	// this->Build_Delaunay3D_Gauss_Mesh(Gauss_Mesh);

	Whether_Gauss_Mesh_Build=1;

	return;
	
}

/*****************************************************************************
* Generate a tetrahedral mesh from the given input points, and stores the 
* newly generated mesh in the same input vtkUnstructuredGrid object.  
*****************************************************************************/

void pvESSI::Build_Delaunay3D_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> GaussMesh){

  vtkSmartPointer<vtkDelaunay3D> delaunay3D = vtkSmartPointer<vtkDelaunay3D>::New();

  vtkDelaunay3D::GlobalWarningDisplayOff();

  delaunay3D->SetInputData (GaussMesh);
  delaunay3D->Update();

  // vtkIndent indent;
  // delaunay3D->PrintSelf(std::cout, indent);

  GaussMesh->ShallowCopy( delaunay3D->GetOutput());

  return;

}

/*****************************************************************************
* This Function creates stress field at nodes by interpolating from 
* gauss points to nodes and then stores it in the field data at nodes folder
*****************************************************************************/
void pvESSI::Build_Stress_Field_At_Nodes_v2(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Node_Mesh_Current_Time){

	// First Check whether Stresses has been calculated or not.
	// If Not Calculated : Calculate it and store it.
	// if Calculated: Visualize it

	Elastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Elastic_Strain"]);

	Plastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Plastic_Strain"]);

	Stress = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Stress"]);

	count1[0]   =1;
	dims1_out[0]=1;
	index_i = (hsize_t)Node_Mesh_Current_Time;

    HDF5_Read_INT_Array_Data(id_Whether_Stress_Strain_Build,
	                          1,
	                         dims1_out,
	                         &index_i,
	                         NULL,
	                         count1,
	                         NULL,
	                         &Int_Variable_1); // Whether_Stress_Strain_Build


	if(Int_Variable_1==-1){

		cout << "<<<<pvESSI>>>> Stress-Strains are not interpolated for this time_step " << Node_Mesh_Current_Time <<endl;
		cout << "<<<<pvESSI>>>> I will calculate and store it for future \n" << endl;

		DataSpace = H5Dget_space (id_Stress_and_Strain);
	    H5Sget_simple_extent_dims (DataSpace, dims3, NULL);
	    H5Sclose(DataSpace);
			
	    dims3[0] = this->Number_of_Nodes;
	    dims3[1] = dims3[1]<Number_Of_Time_Steps?(dims3[1]+1):dims3[1];
	    dims3[2] = 18;
	    status = H5Dset_extent (id_Stress_and_Strain, dims3);

		index_i = -1;
		while(++index_i < Number_of_Elements){

			count1[0]   =1;
			dims1_out[0]=1;

		    HDF5_Read_INT_Array_Data(id_Element_Map,
			                          1,
			                         dims1_out,
			                         &index_i,
			                         NULL,
			                         count1,
			                         NULL,
			                         &Int_Variable_1); // Element_Map

		    index_j = (hsize_t) Int_Variable_1;

		    HDF5_Read_INT_Array_Data(id_Number_of_Gauss_Points,
			                          1,
			                         dims1_out,
			                         &index_j,
			                         NULL,
			                         count1,
			                         NULL,
			                         &Int_Variable_1); // Element_Number_of_Gauss_Nodes

		    HDF5_Read_INT_Array_Data(id_Class_Tags,
			                          1,
			                         dims1_out,
			                         &index_j,
			                         NULL,
			                         count1,
			                         NULL,
			                         &Int_Variable_2); // Element_Class_Tags

			std::map<int,double**>::iterator it;
			it = Gauss_To_Node_Interpolation_Map.find(Int_Variable_2);
			double Stress_Strain_Outputs [18]; 

			if(Int_Variable_1>0){

				if(it != Gauss_To_Node_Interpolation_Map.end()){

				    HDF5_Read_INT_Array_Data(id_Index_to_Outputs,
					                          1,
					                         dims1_out,
					                         &index_j,
					                         NULL,
					                         count1,
					                         NULL,
					                         &Int_Variable_2); // Element_Index_to_Output

				    HDF5_Read_INT_Array_Data(id_Element_Number_of_Nodes,
					                          1,
					                         dims1_out,
					                         &index_j,
					                         NULL,
					                         count1,
					                         NULL,
					                         &Int_Variable_3); // Element_Number_of_Nodes	


				    double **Stress_Strain_At_Nodes = new double*[Int_Variable_3];
				    double **Stress_Strain_At_Gauss_Points = new double*[Int_Variable_3];

				    index_k = -1;
					while(++index_k < Int_Variable_3){
						Stress_Strain_At_Nodes[(int) index_k] = new double[18];
						Stress_Strain_At_Gauss_Points[(int) index_k] = new double[18];
					}

				    index_k = -1;
					while(++index_k < Int_Variable_3){

						offset2[0] = (hsize_t)(Int_Variable_2) + (18*index_k);  offset2[1] = Node_Mesh_Current_Time;
					    count2 [0] = 18;  count2 [1] = 1; 
					    dims1_out[0] =18;


					    HDF5_Read_DOUBLE_Array_Data(id_Outputs,
						                          1,
						                         dims1_out,
						                         offset2,
						                         NULL,
						                         count2,
						                         NULL,
						                         Stress_Strain_Outputs); // Element_Stress_Strain_Outputs_at_Gauss-points

						index = 18;
						while(index--){
							Stress_Strain_At_Gauss_Points[(int) index_k][index]= Stress_Strain_Outputs[index];
						}
					}

					vtkMath::MultiplyMatrix	(it->second,Stress_Strain_At_Gauss_Points,Int_Variable_3,Int_Variable_3,Int_Variable_3,18,Stress_Strain_At_Nodes);

					// for(int i =0; i<Int_Variable_3 ; i++ ){
					// 	for(int j=0; j<18; j++)
					// 		cout << Stress_Strain_At_Nodes[i][j] << " ";
					// 	cout << endl;
					// }
					// cout << endl;
					
					count1[0]   =1;
					dims1_out[0]=1;

				    HDF5_Read_INT_Array_Data(id_Index_to_Connectivity,
					                          1,
					                         dims1_out,
					                         &index_j,
					                         NULL,
					                         count1,
					                         NULL,
					                         &Int_Variable_2); // Element_Index_to_Connectivity

					index = -1;
					while(++index < Int_Variable_3){

						index_k = (hsize_t)(Int_Variable_2) + (hsize_t)index;

						count1[0]   =1;
						dims1_out[0]=1;

					    HDF5_Read_INT_Array_Data(id_Connectivity,
						                          1,
						                         dims1_out,
						                         &index_k,
						                         NULL,
						                         count1,
						                         NULL,
						                         &Int_Variable_4); // Element_Connectivity_Node_No

						index_k = (hsize_t)(Int_Variable_4);

					    HDF5_Read_INT_Array_Data(id_Inverse_Node_Map,
						                          1,
						                         dims1_out,
						                         &index_k,
						                         NULL,
						                         count1,
						                         NULL,
						                         &Int_Variable_4); // Node_Map_Node_No

					    offset3[0] = (hsize_t)(Int_Variable_4);  offset3[1]=dims3[1]-1;    offset3[2] = 0;
					    count3 [0] = 1;							 count3 [1] = 1;		   count3 [2] = 18;
					    dims1_out[0] =18;

					    HDF5_Read_DOUBLE_Array_Data(id_Stress_and_Strain,
						                          3,
						                         dims1_out,
						                         offset3,
						                         NULL,
						                         count3,
						                         NULL,
						                         Stress_Strain_Outputs); // Stress_and_Strain-Prevous_Outputs_Reading

					    for(int i=0; i<18;i++){
					    	Stress_Strain_Outputs[i]=Stress_Strain_Outputs[i]+Stress_Strain_At_Nodes[index][i];
					    }

					    HDF5_Write_DOUBLE_Array_Data(id_Stress_and_Strain,
						                          3,
						                         dims1_out,
						                         offset3,
						                         NULL,
						                         count3,
						                         NULL,
						                         Stress_Strain_Outputs); // Stress_and_Strain-After_Appending__Outputs_Writing_Back

						// for(int j=0; j<18; j++)
						// 	cout << Stress_Strain_Outputs[j] << " ";
						// cout << endl;

					}


				}
				else{

					cout << "<<<<pvESSI>>>> Build_Stress_Field_At_Nodes:: Warning!! Gauss_Interpolation to nodes not implemented for element of Class_Tag " << Int_Variable_2 << endl;

				}

			}

		}

		/********************* Iterate over all nodes and take the average of the stress_strains *******************************************/

		Float_Variable_1 = 1e-5; 					// epsilon

		index_i = -1;
		while(++index_i < Number_of_Nodes){

			double Node_Stress_Strain_Outputs [18];

			count1[0]   =1;
			dims1_out[0]=1;		    
		    HDF5_Read_INT_Array_Data(id_Number_of_Gauss_Elements_Shared,
			                          1,
			                         dims1_out,
			                         &index_i,
			                         NULL,
			                         count1,
			                         NULL,
			                         &Int_Variable_1); // Number_of-Gauss_Elements_Shared	

			offset3[0] = index_i;  					 offset3[1] =dims3[1]-1;    offset3[2] = 0;
		    count3 [0] = 1;							 count3 [1] = 1;		    count3 [2] = 18;
		    dims1_out[0] =18;

		    HDF5_Read_DOUBLE_Array_Data(id_Stress_and_Strain,
			                          3,
			                         dims1_out,
			                         offset3,
			                         NULL,
			                         count3,
			                         NULL,
			                         Node_Stress_Strain_Outputs); // Stress_and_Strain-Prevous_Outputs_Reading	

		    // cout << " Number_of_gauss_Elemnts_Shared by node " << index_i << "is " << Int_Variable_1 <<endl;;

		    // for ( int i=0; i< 18 ; i++){
		    // 	cout << Node_Stress_Strain_Outputs[i] << "  ";
		    // }
		    // cout << endl;

		    index_j=18;
		    while(index_j--){
			    Node_Stress_Strain_Outputs[index_j]=Node_Stress_Strain_Outputs[index_j]/((float)Int_Variable_1 +(float)Float_Variable_1);
		    }

		    // for ( int i=0; i< 18 ; i++){
		    // 	cout << Node_Stress_Strain_Outputs[i] << "  ";
		    // }
		    // cout << endl;

		    HDF5_Write_DOUBLE_Array_Data(id_Stress_and_Strain,
			                          3,
			                         dims1_out,
			                         offset3,
			                         NULL,
			                         count3,
			                         NULL,
			                         Node_Stress_Strain_Outputs); // Stress_and_Strain-Prevous_Outputs_Reading	    

		}

		// Writing back to whether stress starin build
		// the index number of the where the stress_strain if for current time step

		Int_Variable_1 = dims3[1]-1;
		count1[0]   =1;
		dims1_out[0]=1;
		index_i = (hsize_t)Node_Mesh_Current_Time;

	    HDF5_Write_INT_Array_Data(id_Whether_Stress_Strain_Build,
		                          1,
		                         dims1_out,
		                         &index_i,
		                         NULL,
		                         count1,
		                         NULL,
		                         &Int_Variable_1); // Whether_Stress_Strain_Build_Index 

	}

	offset3[0] = index_i;  					 offset3[1] = (hsize_t)Int_Variable_1;    offset3[2] = 0;
    count3 [0] = 1;							 count3 [1] = 1;		    			  count3 [2] = 18;
    dims1_out[0] =18;


	index_i = -1;
	while(++index_i < Number_of_Nodes){

		double Node_Stress_Strain_Outputs [18];

		offset3[0] = index_i;  					 

	    HDF5_Read_DOUBLE_Array_Data(id_Stress_and_Strain,
		                          3,
		                         dims1_out,
		                         offset3,
		                         NULL,
		                         count3,
		                         NULL,
		                         Node_Stress_Strain_Outputs); // Stress_and_Strains_Outputs_Reading	

			float El_Strain_Tuple[6] ={
				Node_Stress_Strain_Outputs[0],
				Node_Stress_Strain_Outputs[3],
				Node_Stress_Strain_Outputs[4],
				Node_Stress_Strain_Outputs[1],
				Node_Stress_Strain_Outputs[5],
				Node_Stress_Strain_Outputs[2]
			};

			float Pl_Strain_Tuple[6] ={
				Node_Stress_Strain_Outputs[6],
				Node_Stress_Strain_Outputs[9],
				Node_Stress_Strain_Outputs[10],
				Node_Stress_Strain_Outputs[7],
				Node_Stress_Strain_Outputs[11],
				Node_Stress_Strain_Outputs[8]
			};

			float Stress_Tuple[6] ={
				Node_Stress_Strain_Outputs[12],
				Node_Stress_Strain_Outputs[15],
				Node_Stress_Strain_Outputs[16],
				Node_Stress_Strain_Outputs[13],
				Node_Stress_Strain_Outputs[17],
				Node_Stress_Strain_Outputs[14]
			};

			Elastic_Strain->InsertTypedTuple ((int)index_i,El_Strain_Tuple);
			Plastic_Strain->InsertTypedTuple ((int)index_i,Pl_Strain_Tuple);
			Stress->InsertTypedTuple ((int)index_i,Stress_Tuple);

	}

	Node_Mesh->GetPointData()->AddArray(Elastic_Strain);
	Node_Mesh->GetPointData()->AddArray(Plastic_Strain);
	Node_Mesh->GetPointData()->AddArray(Stress);

	return;
}


/*****************************************************************************
* This function generates the metadatay attributes array all at one place  
* I am thinking it to remove it. Was meant to be public accessibe to all but
* currently takes lot of parameters. 
*****************************************************************************/

void pvESSI::Set_Meta_Array( int Meta_Array_Id ){

	switch (Meta_Array_Id){

		case 0:

			Generalized_Displacements->SetName("Generalized_Displacements");
			Generalized_Displacements->SetNumberOfComponents(3);
			Generalized_Displacements->SetComponentName(0,"UX");
			Generalized_Displacements->SetComponentName(1,"UY");
			Generalized_Displacements->SetComponentName(2,"UZ");
			break;  

		case 1:

			Generalized_Velocity->SetName("Generalized_Velocity");
			Generalized_Velocity->SetNumberOfComponents(3);
			Generalized_Velocity->SetComponentName(0,"VX");
			Generalized_Velocity->SetComponentName(1,"VY");
			Generalized_Velocity->SetComponentName(2,"VZ");
			break;

		case 2:

			Generalized_Acceleration->SetName("Generalized_Acceleration");
			Generalized_Acceleration->SetNumberOfComponents(3);
			Generalized_Acceleration->SetComponentName(0,"AX");
			Generalized_Acceleration->SetComponentName(1,"AY");
			Generalized_Acceleration->SetComponentName(2,"AZ");
			break;

		case 3:

			Elastic_Strain->SetName("Elastic_Strain");
			Elastic_Strain->SetNumberOfComponents(6);
			Elastic_Strain->SetComponentName(0,"eps_xx");
			Elastic_Strain->SetComponentName(1,"eps_xy");
			Elastic_Strain->SetComponentName(2,"eps_xz");
			Elastic_Strain->SetComponentName(3,"eps_yy");
			Elastic_Strain->SetComponentName(4,"eps_yz");
			Elastic_Strain->SetComponentName(5,"eps_zz");
			break;

		case 4:

			Plastic_Strain->SetName("Plastic_Strain");
			Plastic_Strain->SetNumberOfComponents(6);
			Plastic_Strain->SetComponentName(0,"eps_xx");
			Plastic_Strain->SetComponentName(1,"eps_xy");
			Plastic_Strain->SetComponentName(2,"eps_xz");
			Plastic_Strain->SetComponentName(3,"eps_yy");
			Plastic_Strain->SetComponentName(4,"eps_yz");
			Plastic_Strain->SetComponentName(5,"eps_zz");
			break;

		case 5:

			Stress->SetName("Stress");
			Stress->SetNumberOfComponents(6);
			Stress->SetComponentName(0,"sig_xx");
			Stress->SetComponentName(1,"sig_xy");
			Stress->SetComponentName(2,"sig_xz");
			Stress->SetComponentName(3,"sig_yy");
			Stress->SetComponentName(4,"sig_yz");
			Stress->SetComponentName(5,"sig_zz");
			break;

		case 6:
			Material_Tag->SetName("Material_Tag");
			Material_Tag->SetNumberOfComponents(1);
			break;

		case 7:
			Total_Energy->SetName("Total_Energy");
			Total_Energy->SetNumberOfComponents(1);
			break;

		case 8:
			Incremental_Energy->SetName("Incremental_Energy");
			Incremental_Energy->SetNumberOfComponents(1);
			break;

		case 9:
			Node_Tag->SetName("Node_Tag");
			Node_Tag->SetNumberOfComponents(1);
			break;

		case 10:
			Element_Tag->SetName("Element_Tag");
			Element_Tag->SetNumberOfComponents(1);
			break;
	}

	return;

}

/*****************************************************************************
* Initializes some variables to default values
*****************************************************************************/

void pvESSI::initialize(){

	Node_Mesh_Current_Time=0;
	Gauss_Mesh_Current_Time=0; 
	Display_Node_Mesh=1;
	Display_Gauss_Mesh=0;
	Display_All_Mesh=0;
	Whether_Node_Mesh_Build=0;
	Whether_Gauss_Mesh_Build=0;
	Whether_All_Mesh_Build=0;
	Build_Map_Status = 1;
	// this->DebugOn();
	// cout << "xascaS" << endl;

	//////////////////////////////////////////////////////// Reading Hdf5 File ///////////////////////////////////////////////////////////////////////////////////////

	  /***************** File_id **********************************/
	  this->id_File = H5Fopen(this->FileName, H5F_ACC_RDWR, H5P_DEFAULT);;

	  /***************** Time Steps *******************************/
	  this->id_time = H5Dopen(id_File, "/time", H5P_DEFAULT); 
	  this->id_Number_of_Time_Steps = H5Dopen(id_File, "/Number_of_Time_Steps", H5P_DEFAULT); 

	  /***************** Model Info *******************************/
	  this->id_Model_group = H5Gopen(id_File, "Model", H5P_DEFAULT); 
	  this->id_Whether_Maps_Build = H5Dopen(id_File, "/Whether_Maps_Build", H5P_DEFAULT); 
	  this->id_Number_of_Elements = H5Dopen(id_File, "/Number_of_Elements", H5P_DEFAULT); 
	  this->id_Number_of_Nodes    = H5Dopen(id_File, "/Number_of_Nodes", H5P_DEFAULT);
	  this->id_Number_of_Processes_Used = H5Dopen(id_File, "/Number_of_Processes_Used", H5P_DEFAULT);
	  this->id_Process_Number     = H5Dopen(id_File, "/Process_Number", H5P_DEFAULT);

	  /**************** Element Info ******************************/
	  this->id_Elements_group = H5Gopen(id_File, "/Model/Elements", H5P_DEFAULT); 
	  this->id_Class_Tags  = H5Dopen(id_File, "/Model/Elements/Class_Tags", H5P_DEFAULT);
	  this->id_Connectivity = H5Dopen(id_File, "/Model/Elements/Connectivity", H5P_DEFAULT);
	  this->id_Element_types = H5Dopen(id_File, "/Model/Elements/Element_types", H5P_DEFAULT);
	  this->id_Gauss_Point_Coordinates = H5Dopen(id_File, "Model/Elements/Gauss_Point_Coordinates", H5P_DEFAULT);
	  this->id_Index_to_Connectivity = H5Dopen(id_File, "Model/Elements/Index_to_Connectivity", H5P_DEFAULT);
	  this->id_Index_to_Gauss_Point_Coordinates = H5Dopen(id_File, "Model/Elements/Index_to_Gauss_Point_Coordinates", H5P_DEFAULT);
	  this->id_Index_to_Outputs = H5Dopen(id_File, "Model/Elements/Index_to_Outputs", H5P_DEFAULT);
	  this->id_Material_tags = H5Dopen(id_File, "Model/Elements/Material_tags", H5P_DEFAULT);
	  this->id_Number_of_Gauss_Points = H5Dopen(id_File, "Model/Elements/Number_of_Gauss_Points", H5P_DEFAULT);
	  this->id_Element_Number_of_Nodes = H5Dopen(id_File, "Model/Elements/Number_of_Nodes", H5P_DEFAULT);
	  this->id_Number_of_Output_Fields = H5Dopen(id_File, "Model/Elements/Number_of_Output_Fields", H5P_DEFAULT);
	  this->id_Outputs = H5Dopen(id_File, "Model/Elements/Outputs", H5P_DEFAULT);
	  // this->id_Substep_Outputs = H5Dopen(id_File, "Model/Elements/Substep_Outputs", H5P_DEFAULT);

	  /**************** Node Info ******************************/
	  this->id_Nodes_group = H5Gopen(id_File, "/Model/Nodes", H5P_DEFAULT); 
	  this->id_Constrained_DOFs = H5Dopen(id_File, "Model/Nodes/Constrained_DOFs", H5P_DEFAULT);
	  this->id_Constarined_Nodes = H5Dopen(id_File, "Model/Nodes/Constrained_Nodes", H5P_DEFAULT);
	  this->id_Coordinates = H5Dopen(id_File, "Model/Nodes/Coordinates", H5P_DEFAULT);
	  this->id_Generalized_Displacements = H5Dopen(id_File, "Model/Nodes/Generalized_Displacements", H5P_DEFAULT);
	  this->id_Index_to_Coordinates = H5Dopen(id_File, "Model/Nodes/Index_to_Coordinates", H5P_DEFAULT);
	  this->id_Index_to_Generalized_Displacements = H5Dopen(id_File, "Model/Nodes/Index_to_Generalized_Displacements", H5P_DEFAULT);
	  this->id_Number_of_DOFs= H5Dopen(id_File, "Model/Nodes/Number_of_DOFs", H5P_DEFAULT);

	  /**************** Maps ***********************************/
	  this->id_Maps_group  = H5Gopen(id_File, "/Maps", H5P_DEFAULT); 
	  this->id_Element_Map = H5Dopen(id_File, "Maps/Element_Map", H5P_DEFAULT);
	  this->id_Node_Map = H5Dopen(id_File, "Maps/Node_Map", H5P_DEFAULT);
	  this->id_Inverse_Node_Map = H5Dopen(id_File, "Maps/Inverse_Node_Map", H5P_DEFAULT);
	  this->id_Inverse_Element_Map = H5Dopen(id_File, "Maps/Inverse_Element_Map", H5P_DEFAULT);
	  this->id_Number_of_Elements_Shared = H5Dopen(id_File, "Maps/Number_of_Elements_Shared", H5P_DEFAULT);
	  this->id_Number_of_Gauss_Elements_Shared = H5Dopen(id_File, "Maps/Number_of_Gauss_Elements_Shared", H5P_DEFAULT);

	  /*************** Field at Nodes ***************************/
	  this->id_Field_at_Nodes_group = H5Gopen(id_File, "/Field_at_Nodes", H5P_DEFAULT); 
	  this->id_Stress_and_Strain = H5Dopen(id_File, "/Field_at_Nodes/Stress_And_Strain", H5P_DEFAULT);
	  this->id_Whether_Stress_Strain_Build = H5Dopen(id_File, "/Field_at_Nodes/Whether_Stress_Strain_Build", H5P_DEFAULT);
	  // this->id_Energy = H5Dopen(id_File, "/Field_at_Nodes/Energy", H5P_DEFAULT);                                         // Not implemented
	  // this->id_Whether_Energy_Build = H5Dopen(id_File, "/Field_at_Nodes/Whether_Energy_Build", H5P_DEFAULT);             // Not implemented

}


/*****************************************************************************
* Builds maps in hdf5 output file for efficient visualization
* Creates a maps group in the files with the following data set 
*	Node_Map: From global/(input) node number to reduced node numbers
*	Element_Map : From global(input) element number to reduced element numbers
* 	Inverse_Node_Map: From reduced node number to global(input) node number 
* 	Inverse_Element_Map: From reduced element number to global(input) element number 
*****************************************************************************/

void pvESSI::Build_Maps(){

	/*********************************************** First Need to build Datasets and Folders *********************************************************************/

	id_Maps_group = H5Gcreate(id_File, "/Maps", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
	H5Gclose(id_Maps_group); 

	id_Field_at_Nodes_group = H5Gcreate(id_File, "/Field_at_Nodes", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
	H5Gclose(id_Field_at_Nodes_group); 

	dims1_out[0]= Number_of_Nodes;

	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Node_Map = H5Dcreate(id_File,"Maps/Node_Map",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Number_of_Elements_Shared = H5Dcreate(id_File,"Maps/Number_of_Elements_Shared",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Number_of_Gauss_Elements_Shared = H5Dcreate(id_File,"Maps/Number_of_Gauss_Elements_Shared",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	dims1_out[0]= Number_of_Elements; 
	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Element_Map = H5Dcreate(id_File,"Maps/Element_Map",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	dims1_out[0]= Pseudo_Number_of_Nodes; 
	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Inverse_Node_Map = H5Dcreate(id_File,"Maps/Inverse_Node_Map",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	dims1_out[0]= Pseudo_Number_of_Elements; 
	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Inverse_Element_Map = H5Dcreate(id_File,"Maps/Inverse_Element_Map",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	/************************************************* Write default Values of -1 for all time steps *****************************************/
	dims1_out[0]= this->Number_Of_Time_Steps; 
	DataSpace = H5Screate_simple(1, dims1_out, NULL);
	id_Whether_Stress_Strain_Build = H5Dcreate(id_File,"Field_at_Nodes/Whether_Stress_Strain_Build",H5T_NATIVE_INT,DataSpace,H5P_DEFAULT,H5P_DEFAULT, H5P_DEFAULT);
	H5Sclose(DataSpace);

	int Whether_Stress_Strain_Build[Number_Of_Time_Steps];
	index = -1;
	while(++index<Number_Of_Time_Steps){
		Whether_Stress_Strain_Build[index]=-1;
	}

	count1[0]   =Number_Of_Time_Steps;
	dims1_out[0]=Number_Of_Time_Steps;
	index_i     =0;

    HDF5_Write_INT_Array_Data(id_Whether_Stress_Strain_Build,
	                          1,
	                         dims1_out,
	                         &index_i,
	                         NULL,
	                         count1,
	                         NULL,
	                         Whether_Stress_Strain_Build); // Whether_Stress_Strain_Build


	/****************************************************************************************************************************************/
	dims3[0]=this->Number_of_Nodes;       dims3[1]= 0;                               dims3[2]= 18; 
	maxdims3[0]=this->Number_of_Nodes; maxdims3[1]= this->Number_Of_Time_Steps;   maxdims3[2]= 18;
	DataSpace = H5Screate_simple(3, dims3, maxdims3);

    hid_t prop = H5Pcreate (H5P_DATASET_CREATE);
    hsize_t chunk_dims[3] = {this->Number_of_Nodes,1,18};
    H5Pset_chunk (prop, 3, chunk_dims);
	id_Stress_and_Strain = H5Dcreate(id_File,"Field_at_Nodes/Stress_And_Strain",H5T_NATIVE_DOUBLE,DataSpace,H5P_DEFAULT,prop, H5P_DEFAULT); 
	status = H5Sclose(DataSpace);

	count1[0]   =1;
	dims1_out[0]=1;

	index_i=-1;
	index  = 0;

	while(++index_i < this->Pseudo_Number_of_Nodes){

	     HDF5_Read_INT_Array_Data(id_Index_to_Coordinates,
			                        1,
			                       dims1_out,
			                       &index_i,
			                       NULL,
			                       count1,
			                       NULL,
			                       &Int_Variable_1); // Index_to_coordinates

		if(Int_Variable_1 !=-1){

			index_j=(hsize_t) index;

		     HDF5_Write_INT_Array_Data(id_Node_Map,
				                        1,
				                       dims1_out,
				                       &index_j,
				                       NULL,
				                       count1,
				                       NULL,
				                       (int*) &index_i); // Node_Number_from_PARAVIEW_to__ESSI

		     HDF5_Write_INT_Array_Data(id_Inverse_Node_Map,
				                        1,
				                       dims1_out,
				                       &index_i,
				                       NULL,
				                       count1,
				                       NULL,
				                       &index);			// Node_Number_from_ESSI_to_PARAVIEW

			index = index+1;
		}
		else{
		     HDF5_Write_INT_Array_Data(id_Inverse_Node_Map,
				                        1,
				                       dims1_out,
				                       &index_i,
				                       NULL,
				                       count1,
				                       NULL,
				                       &Int_Variable_1); // Node_Number_from_ESSI_to_PARAVIEW
		}
	}

	index_i=-1;
	index =0;

	std::map<int,double**>::iterator it;

	while(++index_i < this->Pseudo_Number_of_Elements){

	     HDF5_Read_INT_Array_Data(id_Index_to_Connectivity,
			                        1,
			                       dims1_out,
			                       &index_i,
			                       NULL,
			                       count1,
			                       NULL,
			                       &Int_Variable_1); // Index_to_connectivity

		if(Int_Variable_1 !=-1){

			// cout << " Pseudo Element_Number " << Int_Variable_1 <<endl;

			index_j=(hsize_t) index;

		     HDF5_Write_INT_Array_Data(id_Element_Map,
				                        1,
				                       dims1_out,
				                       &index_j,
				                       NULL,
				                       count1,
				                       NULL,
				                       (int*) &index_i); // Element_Number_from_PARAVIEW_to__ESSI

		     HDF5_Write_INT_Array_Data(id_Inverse_Element_Map,
				                        1,
				                       dims1_out,
				                       &index_i,
				                       NULL,
				                       count1,
				                       NULL,
				                       &index); //Element_Number_from_ESSI_to_PARAVIEW

		     HDF5_Read_INT_Array_Data(id_Number_of_Gauss_Points,
				                        1,
				                       dims1_out,
				                       &index_i,
				                       NULL,
				                       count1,
				                       NULL,
				                       &Int_Variable_2); // Element_Number_of_Gauss_Points

		     if(Int_Variable_2>0){

 				   HDF5_Read_INT_Array_Data(id_Class_Tags,
						                    1,
						                   dims1_out,
						                   &index_i,
						                   NULL,
						                   count1,
						                   NULL,
						                   &Int_Variable_2); // Element_Class_Tags
		     		
				   it = Gauss_To_Node_Interpolation_Map.find(Int_Variable_2);

				   if(it != Gauss_To_Node_Interpolation_Map.end()){

				   		this->Append_Number_of_Elements_Shared(1);

				   }
				   else{
				   	   cout << "<<<<[pvESSI]>>>> Build_Maps:: Warning!! Gauss to Node Interpolation not implemented for element of Class_Tag  " << Int_Variable_2 << endl;
				   	   this->Append_Number_of_Elements_Shared(0);
				   }



		     }
		     else{

		     	this->Append_Number_of_Elements_Shared(0);

		     }

			index = index+1;
		}
		else{

		     HDF5_Write_INT_Array_Data(id_Inverse_Element_Map,
		                        1,
		                       dims1_out,
		                       &index_i,
		                       NULL,
		                       count1,
		                       NULL,
		                       &Int_Variable_1);//Element_Number_from_ESSI_to_PARAVIEW
		}
	}

}

void pvESSI::Append_Number_of_Elements_Shared(int message)  {


	//index_i -> ESSI_Element_Number from Build_Maps

	HDF5_Read_INT_Array_Data(id_Element_Number_of_Nodes,
			                    1,
			                   dims1_out,
			                   &index_i,
			                   NULL,
			                   count1,
			                   NULL,
			                   &Int_Variable_2); // Element_Number_of_Nodes

	index_j= -1;
	while(++index_j <Int_Variable_2){

		index_k = (hsize_t) Int_Variable_1 + index_j;

	    HDF5_Read_INT_Array_Data(id_Connectivity,
                    1,
                   dims1_out,
                   &index_k,
                   NULL,
                   count1,
                   NULL,
                   &Int_Variable_3); // Element_Connectivty_Node

	    index_k = (hsize_t) Int_Variable_3;

	    HDF5_Read_INT_Array_Data(id_Inverse_Node_Map,
                    1,
                   dims1_out,
                   &index_k,
                   NULL,
                   count1,
                   NULL,
                   &Int_Variable_3); // Inverse_Node_Map

		index_k = (hsize_t) Int_Variable_3;

	    HDF5_Read_INT_Array_Data(id_Number_of_Elements_Shared,
	                        1,
	                       dims1_out,
	                       &index_k,
	                       NULL,
	                       count1,
	                       NULL,
	                       &Int_Variable_3); // Number_of_Elements_Shared_Per_Node

	    Int_Variable_3 = Int_Variable_3 +1;

   	    HDF5_Write_INT_Array_Data(id_Number_of_Elements_Shared,
                        1,
                       dims1_out,
                       &index_k,
                       NULL,
                       count1,
                       NULL,
                       &Int_Variable_3); // Number_of_Elements_Shared_Per_Node 

   	    if(message==1){

		    HDF5_Read_INT_Array_Data(id_Number_of_Gauss_Elements_Shared,
		                        1,
		                       dims1_out,
		                       &index_k,
		                       NULL,
		                       count1,
		                       NULL,
		                       &Int_Variable_3); // Number_of_Gauss_Elements_Shared_Per_Node

		    Int_Variable_3 = Int_Variable_3 +1;

	   	    HDF5_Write_INT_Array_Data(id_Number_of_Gauss_Elements_Shared,
	                        1,
	                       dims1_out,
	                       &index_k,
	                       NULL,
	                       count1,
	                       NULL,
	                       &Int_Variable_3); // Number_of_Gauss_Elements_Shared_Per_Node 
   	    }

	}

	return;


}


/*****************************************************************************
* Builds time map i.e. a way to get the index number from a given non-interger
* time request from paraview desk. Although it works fine, its not the great 
* way to achieve it. 
* Needs to be changed. I think, paraview has fixed this issue in their latest 
* version, but I am not sure, need to check it. 
*****************************************************************************/

void pvESSI::Build_Time_Map(){

	for (int i = 0;i<Number_Of_Time_Steps ; i++)
		Time_Map[Time[i]] = i;

	return;
}


/*****************************************************************************
* Builds Meta Array map, so that it is easie to add attributes to vtkObject 
* and is neat and clean.
*****************************************************************************/

void pvESSI::Build_Meta_Array_Map(){

	int key = 0;

	Meta_Array_Map["Generalized_Displacements"] = key; key=key+1;
	Meta_Array_Map["Generalized_Velocity"] = key; key=key+1;
	Meta_Array_Map["Generalized_Acceleration"] = key; key=key+1;
	Meta_Array_Map["Elastic_Strain"] = key; key=key+1;
	Meta_Array_Map["Plastic_Strain"] = key; key=key+1;
	Meta_Array_Map["Stress"] = key; key=key+1;
	Meta_Array_Map["Material_Tag"] = key; key=key+1;
	Meta_Array_Map["Total_Energy"] = key; key=key+1;
	Meta_Array_Map["Incremental_Energy"] = key; key=key+1;
	Meta_Array_Map["Node_Tag"] = key; key=key+1;
	Meta_Array_Map["Element_Tag"] = key; key=key+1;

}

void pvESSI::Build_Inverse_Matrices(){

	Build_Brick_Coordinates();

	double SQRT_3 = sqrt(3);
	double SQRT_3_5 = sqrt(0.6);

	/////////////////////////////////////////// Building 8_Node_Brick_Inverse ///////////////////////////////////////

	this->Eight_Node_Brick_Inverse = new double*[8];

	double **Temp_Eight_Node_Brick = new double*[8];
	for(int i = 0; i < 8; ++i){
    	Temp_Eight_Node_Brick[i] = new double[8];
    	Eight_Node_Brick_Inverse[i] = new double[8];
	}

	for (int j=0; j<8;j++){
		for (int i=0; i<8;i++){
			Temp_Eight_Node_Brick[j][i] =   0.125*(1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3)*
											      (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3)*
											      (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3);
		}
	}

	vtkMath::InvertMatrix(Temp_Eight_Node_Brick,Eight_Node_Brick_Inverse,8);

	//////////////////////////////////////// Building 20_Node_Brick_Inverse //////////////////////////////////////

  	this->Twenty_Node_Brick_Inverse = new double*[20];
	double **Temp_Twenty_Node_Brick = new double*[20];
	for(int i = 0; i < 20; ++i){
    	Temp_Twenty_Node_Brick[i] = new double[20];
    	Twenty_Node_Brick_Inverse[i] = new double[20];
	}
	for (int j=0; j<20;j++){
		for (int i=0; i<8;i++){
			Temp_Twenty_Node_Brick[j][i] =   0.125*(1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)* 
												   (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)* 
												   (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5)* 
												   (-2+ (Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)+ 
												        (Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)+ 
												        (Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5));
		}
		int x[] = { 8,10,12,14};
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Node_Brick[j][i] = 1.0/4.0*(1-pow((Brick_Coordinates[j][0]*SQRT_3_5),2))*
												   (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												   (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5);
		}
		x[0] = 9; x[1]=11; x[2]=13; x[3]=15;
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Node_Brick[j][i] = 1.0/4.0*(1-pow((Brick_Coordinates[j][1]*SQRT_3_5),2))*
												   (1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												   (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5);
		}
		x[0]=16; x[1]=17; x[2]=18; x[3]=19;
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Node_Brick[j][i] = 1.0/4.0*(1-pow((Brick_Coordinates[j][2]*SQRT_3_5),2))*
												   (1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												   (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5);
		}

	}

	vtkMath::InvertMatrix(Temp_Twenty_Node_Brick,Twenty_Node_Brick_Inverse,20);

	////////////////////////////////////////// Building 27_Node_Brick_Inverse //////////////////////////////////////

  	this->Twenty_Seven_Node_Brick_Inverse = new double*[27];
	double **Temp_Twenty_Seven_Brick = new double*[27];
	for(int i = 0; i < 27; ++i){
    	Temp_Twenty_Seven_Brick[i] = new double[27];
    	Twenty_Seven_Node_Brick_Inverse[i] = new double[27];
	}
	for (int j=0; j<27;j++){
		for (int i=0; i<8;i++){
			Temp_Twenty_Seven_Brick[j][i]         = 0.125*(1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												         (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												         (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5)*
												         ((Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												         (Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												         (Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5));
		}
		int x[] = { 8,10,12,14};
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.25*(1-pow((Brick_Coordinates[j][0]*SQRT_3_5),2))*
												        (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												        (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5)*
												        ((Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												        (Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5));
		}
		x[0] = 9; x[1]=11; x[2]=13; x[3]=15;
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.25*(1-pow((Brick_Coordinates[j][1]*SQRT_3_5),2))*
												        (1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												        (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5)*
												        ((Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												        (Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5));
		}
		x[0]=16; x[1]=17; x[2]=18; x[3]=19;
		for (int k=0; k<4;k++){
			int i = x[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.25*(1-pow((Brick_Coordinates[j][2]*SQRT_3_5),2))*
												        (1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												        (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
												        ((Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
												        (Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5));
		}
		{
			int i =20;
			Temp_Twenty_Seven_Brick[j][i]         = 0.25*(1-pow((Brick_Coordinates[j][0]*SQRT_3_5),2))*
													    (1-pow((Brick_Coordinates[j][1]*SQRT_3_5),2))*
													    (1-pow((Brick_Coordinates[j][2]*SQRT_3_5),2));
		}
		int y[] = {21,23};
		for (int k=0; k<2;k++){
			int i = y[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.5*(1-pow((Brick_Coordinates[j][0]*SQRT_3_5),2))*
													   (1-pow((Brick_Coordinates[j][2]*SQRT_3_5),2))*
													   (1+Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5)*
													   (Brick_Coordinates[j][1]*Brick_Coordinates[i][1]*SQRT_3_5);
		}
		y[0]=22; y[1]=24;
		for (int k=0; k<2;k++){
			int i = y[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.5*(1-pow((Brick_Coordinates[j][1]*SQRT_3_5),2))*
													   (1-pow((Brick_Coordinates[j][2]*SQRT_3_5),2))*
													   (1+Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5)*
													   (Brick_Coordinates[j][0]*Brick_Coordinates[i][0]*SQRT_3_5);
		}
		y[0]=25; y[1]=26;
		for (int k=0; k<2;k++){
			int i = y[k];
			Temp_Twenty_Seven_Brick[j][i]         = 0.5*(1-pow((Brick_Coordinates[j][0]*SQRT_3_5),2))*
													   (1-pow((Brick_Coordinates[j][1]*SQRT_3_5),2))*
													   (1+Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5)*
													   (Brick_Coordinates[j][2]*Brick_Coordinates[i][2]*SQRT_3_5);
		}
	}

	vtkMath::InvertMatrix(Temp_Twenty_Seven_Brick,Twenty_Seven_Node_Brick_Inverse,27);

	///////////////////////////// Printing For Debugging //////////////////////////////////////////////////////////

	// for (int j=0; j<27;j++){
	// 	for (int i=0; i<27;i++){
	// 		cout << std::setw(8) << std::setprecision(5) << Twenty_Seven_Node_Brick_Inverse[j][i] << " " ;
	// 	}

	// 	cout << endl;
	// }

	free(Temp_Twenty_Seven_Brick);
	free(Temp_Twenty_Node_Brick);
	free(Temp_Eight_Node_Brick);


	return;
  }

void pvESSI::Build_Brick_Coordinates(){

	Brick_Coordinates[0 ][0]= 1;  Brick_Coordinates[0 ][1]= 1; Brick_Coordinates[0 ][2]= 1;
	Brick_Coordinates[1 ][0]=-1;  Brick_Coordinates[1 ][1]= 1; Brick_Coordinates[1 ][2]= 1;
	Brick_Coordinates[2 ][0]=-1;  Brick_Coordinates[2 ][1]=-1; Brick_Coordinates[2 ][2]= 1;
	Brick_Coordinates[3 ][0]= 1;  Brick_Coordinates[3 ][1]=-1; Brick_Coordinates[3 ][2]= 1;
	Brick_Coordinates[4 ][0]= 1;  Brick_Coordinates[4 ][1]= 1; Brick_Coordinates[4 ][2]=-1;
	Brick_Coordinates[5 ][0]=-1;  Brick_Coordinates[5 ][1]= 1; Brick_Coordinates[5 ][2]=-1;
	Brick_Coordinates[6 ][0]=-1;  Brick_Coordinates[6 ][1]=-1; Brick_Coordinates[6 ][2]=-1;
	Brick_Coordinates[7 ][0]= 1;  Brick_Coordinates[7 ][1]=-1; Brick_Coordinates[7 ][2]=-1;
	Brick_Coordinates[8 ][0]= 0;  Brick_Coordinates[8 ][1]= 1; Brick_Coordinates[8 ][2]= 1;
	Brick_Coordinates[9 ][0]=-1;  Brick_Coordinates[9 ][1]= 0; Brick_Coordinates[9 ][2]= 1;
	Brick_Coordinates[10][0]= 0;  Brick_Coordinates[10][1]=-1; Brick_Coordinates[10][2]= 1;
	Brick_Coordinates[11][0]= 1;  Brick_Coordinates[11][1]= 0; Brick_Coordinates[11][2]= 1;
	Brick_Coordinates[12][0]= 0;  Brick_Coordinates[12][1]= 1; Brick_Coordinates[12][2]=-1;
	Brick_Coordinates[13][0]=-1;  Brick_Coordinates[13][1]= 0; Brick_Coordinates[13][2]=-1;
	Brick_Coordinates[14][0]= 0;  Brick_Coordinates[14][1]=-1; Brick_Coordinates[14][2]=-1;
	Brick_Coordinates[15][0]= 1;  Brick_Coordinates[15][1]= 0; Brick_Coordinates[15][2]=-1;
	Brick_Coordinates[16][0]= 1;  Brick_Coordinates[16][1]= 1; Brick_Coordinates[16][2]= 0;
	Brick_Coordinates[17][0]=-1;  Brick_Coordinates[17][1]= 1; Brick_Coordinates[17][2]= 0;
	Brick_Coordinates[18][0]=-1;  Brick_Coordinates[18][1]=-1; Brick_Coordinates[18][2]= 0;
	Brick_Coordinates[19][0]= 1;  Brick_Coordinates[19][1]=-1; Brick_Coordinates[19][2]= 0;
	Brick_Coordinates[20][0]= 0;  Brick_Coordinates[20][1]= 0; Brick_Coordinates[20][2]= 0;
	Brick_Coordinates[21][0]= 0;  Brick_Coordinates[21][1]= 1; Brick_Coordinates[21][2]= 0;
	Brick_Coordinates[22][0]=-1;  Brick_Coordinates[22][1]= 0; Brick_Coordinates[22][2]= 0;
	Brick_Coordinates[23][0]= 0;  Brick_Coordinates[23][1]=-1; Brick_Coordinates[23][2]= 0;
	Brick_Coordinates[24][0]= 1;  Brick_Coordinates[24][1]= 0; Brick_Coordinates[24][2]= 0;
	Brick_Coordinates[25][0]= 0;  Brick_Coordinates[25][1]= 0; Brick_Coordinates[25][2]= 1;
	Brick_Coordinates[26][0]= 0;  Brick_Coordinates[26][1]= 0; Brick_Coordinates[26][2]=-1;
	return;

}


void pvESSI::Build_Gauss_To_Node_Interpolation_Map(){

	Gauss_To_Node_Interpolation_Map[8001] = this->Eight_Node_Brick_Inverse;
	Gauss_To_Node_Interpolation_Map[8005] = this->Twenty_Node_Brick_Inverse;
	Gauss_To_Node_Interpolation_Map[8002] = this->Twenty_Seven_Node_Brick_Inverse;
}

void pvESSI::HDF5_Read_INT_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       int* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dread(
	      id_DataSet,              // Dataset to write to
	      H5T_NATIVE_INT,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}


void pvESSI::HDF5_Write_INT_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       int* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dwrite(
	      id_DataSet,              // Dataset to write to
	      H5T_NATIVE_INT,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}


void pvESSI::HDF5_Read_FLOAT_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       float* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dread(
	      id_DataSet,              // Dataset to write to
	      H5T_NATIVE_FLOAT,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}

void pvESSI::HDF5_Write_FLOAT_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       float* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dwrite(
	      id_DataSet,        // Dataset to write to
	      H5T_NATIVE_FLOAT,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}


void pvESSI::HDF5_Read_DOUBLE_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       double* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dread(
	      id_DataSet,        // Dataset to write to
	      H5T_NATIVE_DOUBLE,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}


void pvESSI::HDF5_Write_DOUBLE_Array_Data(hid_t id_DataSet,
				                       int rank,
				                       hsize_t *data_dims,
				                       hsize_t *offset,
				                       hsize_t *stride,
				                       hsize_t *count,
				                       hsize_t *block,
				                       double* data)
{

  //Get pointer to the dataspace and create the memory space
  DataSpace = H5Dget_space(id_DataSet);
  MemSpace  = H5Screate_simple(1, data_dims, data_dims);

  //Select the region of data to output to
  H5Sselect_hyperslab(
      DataSpace,             // Id of the parent dataspace
      H5S_SELECT_SET,        // Selection operatior H5S_SELECT_<>, where <> = {SET, OR, AND, XOR, NOTB, NOTA}
      offset,                // start of selection
      stride,                // stride in each dimension, NULL  is select everything
      count ,                // how many blocks to select in each direction
      block                  // little block selected per selection
  );

  H5Dwrite(
	      id_DataSet,        // Dataset to write to
	      H5T_NATIVE_DOUBLE,     // Format of data in memory
	      MemSpace,           // Description of data in memory
	      DataSpace,          // Description of data in storage (including selection)
	      H5P_DEFAULT,         // Form of reading
	      data                // The actual data
	  );

  H5Sclose(DataSpace);
  H5Sclose(MemSpace);
}

void pvESSI::Build_Stress_Field_At_Nodes(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Node_Mesh_Current_Time){

	// First Check whether Stresses has been calculated or not.
	// If Not Calculated : Calculate it and store it.
	// if Calculated: Visualize it

	Elastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Elastic_Strain"]);

	Plastic_Strain = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Plastic_Strain"]);

	Stress = vtkSmartPointer<vtkFloatArray> ::New();
	this->Set_Meta_Array (Meta_Array_Map["Stress"]);

	count1[0]   =1;
	dims1_out[0]=1;
	index_i = (hsize_t)Node_Mesh_Current_Time;

    HDF5_Read_INT_Array_Data(id_Whether_Stress_Strain_Build,
	                          1,
	                         dims1_out,
	                         &index_i,
	                         NULL,
	                         count1,
	                         NULL,
	                         &Int_Variable_1); // Whether_Stress_Strain_Build

	double Node_Stress_And_Strain_Field[this->Number_of_Nodes][18];

	cout << "<<<<pvESSI>>>> Stress-Strains are not interpolated for this time_step " << Node_Mesh_Current_Time <<endl;

	if(Int_Variable_1==-1){

		cout << "<<<<pvESSI>>>> Stress-Strains are not interpolated for this time_step " << Node_Mesh_Current_Time <<endl;
		cout << "<<<<pvESSI>>>> I will calculate and store it for future \n" << endl;

		//////////////////////////////////////////////// Extending the dataset /////////////////////////////////////////////////////////////////////////////
		
		DataSpace = H5Dget_space (id_Stress_and_Strain);
	    H5Sget_simple_extent_dims (DataSpace, dims3, NULL);
		H5Sclose(DataSpace);
		
	    dims3[0] = this->Number_of_Nodes;
	    dims3[1] = dims3[1]<Number_Of_Time_Steps?(dims3[1]+1):dims3[1];
	    dims3[2] = 18;
	    status = H5Dset_extent (id_Stress_and_Strain, dims3);

		offset3[0] = 0;  					     offset3[1] =dims3[1]-1;    offset3[2] = 0;
	    count3 [0] = this->Number_of_Nodes;		 count3 [1] = 1;		    count3 [2] = 18;
	    dims2_out[0] =this->Number_of_Nodes;	 dims2_out[1] = 18;

	    DataSpace = H5Dget_space(id_Stress_and_Strain);
	    MemSpace = H5Screate_simple(2,dims2_out,NULL);
	    H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset3,NULL,count3,NULL);
	    H5Dread(id_Stress_and_Strain, H5T_NATIVE_DOUBLE, MemSpace, DataSpace, H5P_DEFAULT, Node_Stress_And_Strain_Field); 
	    H5Sclose(MemSpace); status=H5Sclose(DataSpace);
		
		//////////////////////////////////////////////////// Reading Map Data ///////////////////////////////////////////////////////////////////////////////////////////

		int Node_Map[Number_of_Nodes];
		H5Dread(id_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Node_Map);

		int Number_of_Gauss_Elements_Shared[Number_of_Nodes];
		H5Dread(id_Number_of_Gauss_Elements_Shared, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Number_of_Gauss_Elements_Shared);

		int Inverse_Node_Map[Pseudo_Number_of_Nodes];
		H5Dread(id_Inverse_Node_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Inverse_Node_Map);

		int Element_Map[Number_of_Elements];
		H5Dread(id_Element_Map, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Map);

		//////////////////////////////////////////////////// Reading Element Data ///////////////////////////////////////////////////////////////////////////////////////////

		int Element_Index_to_Connectivity[Pseudo_Number_of_Elements]; 
	    H5Dread(id_Index_to_Connectivity, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,H5P_DEFAULT, Element_Index_to_Connectivity);

	    int Element_Class_Tags[Pseudo_Number_of_Elements]; 
		H5Dread(id_Class_Tags, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Class_Tags);

	    int Element_Number_of_Gauss_Points[Pseudo_Number_of_Elements]; 
		H5Dread(id_Number_of_Gauss_Points, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Gauss_Points);

		int Element_Number_of_Nodes[Pseudo_Number_of_Elements];
		H5Dread(id_Element_Number_of_Nodes, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Number_of_Nodes); 

		DataSpace = H5Dget_space(id_Connectivity); 
		H5Sget_simple_extent_dims(DataSpace, dims1_out, NULL); status=H5Sclose(DataSpace);	
		int Element_Connectivity[dims1_out[0]];
		H5Dread(id_Connectivity, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Connectivity); 

		////////////////////////////////////////////////////// Reading Element  Attributes /////////////////////////////////////////////////////////////////////////////

		int Element_Index_to_Outputs[Pseudo_Number_of_Elements];
		H5Dread(id_Index_to_Outputs, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,Element_Index_to_Outputs); 

		///////////////////////////////////////////  Output Dataset for a particular time /////////////////////////////////////////////////////////////////////////////	

		DataSpace = H5Dget_space(id_Outputs);
		H5Sget_simple_extent_dims(DataSpace, dims2_out, NULL);	
		float Element_Outputs[dims2_out[0]];
		offset2[0]=0; 					    count2[0] = dims2_out[0];		dims1_out[0]=dims2_out[0];
		offset2[1]=Node_Mesh_Current_Time;  count2[1] = 1;					MemSpace = H5Screate_simple(1,dims1_out,NULL);
		H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset2,NULL,count2,NULL);
		H5Dread(id_Outputs, H5T_NATIVE_FLOAT, MemSpace, DataSpace, H5P_DEFAULT, Element_Outputs); 
		H5Sclose(MemSpace); status=H5Sclose(DataSpace);

		///////////////////////////////////////////// Need to Extend the Dataset ////////////////////////////////////////////////////////////////////////////////////////////
		
		// for(int i =0 ; i<this->Number_of_Nodes;i++){
		// 	for(int j=0;j<18;j++)
		// 		Node_Stress_And_Strain_Field[i][j]=0;
		// 		// cout <<  Node_Stress_And_Strain_Field[i][j] << " " ;
		// 	cout << endl;
		// }

		int connectivity_index,number_of_element_nodes, no_of_gauss_points, element_no, Class_Tags;  
		for (int i = 0; i < this->Number_of_Elements; ++i)
		{
			element_no = Element_Map[i];
			Class_Tags = Element_Class_Tags[element_no];
			no_of_gauss_points = Element_Number_of_Gauss_Points[element_no];
			std::map<int,double**>::iterator it;
			it = Gauss_To_Node_Interpolation_Map.find(Class_Tags);

			if(no_of_gauss_points>0){

				if(it != Gauss_To_Node_Interpolation_Map.end()){

					connectivity_index = Element_Index_to_Connectivity[element_no]; 

					number_of_element_nodes = Element_Number_of_Nodes[element_no];

					//////////////////////// Initializing Containers ///////////////////////////////////////
					double **Stress_Strain_At_Nodes = new double*[number_of_element_nodes];
				    double **Stress_Strain_At_Gauss_Points = new double*[number_of_element_nodes];					
					for(int j = 0; j < number_of_element_nodes; ++j){
				    	Stress_Strain_At_Nodes[j] = new double[18];
				    	Stress_Strain_At_Gauss_Points[j] = new double[18];
					}

					/////////////////////// Calculating Stresses at Nodes /////////////////////////////////

					int output_index = Element_Index_to_Outputs[element_no];

					for(int j=0; j< number_of_element_nodes ; j++){
						for(int k=0; k< 18 ; k++){
							Stress_Strain_At_Gauss_Points[j][k] = Element_Outputs[output_index+j*18+k];
						}
					}

					// for(int p =0; p<number_of_element_nodes ; p++ ){
					// 	for(int q=0; q<18; q++)
					// 		cout << Stress_Strain_At_Gauss_Points[p][q] << " ";
					// 	cout << endl;
					// }
					// cout << endl;

					vtkMath::MultiplyMatrix	(it->second,Stress_Strain_At_Gauss_Points,number_of_element_nodes,number_of_element_nodes,number_of_element_nodes,18,Stress_Strain_At_Nodes);

					///////////////////////// Adding the Calculated Stresses at Nodes //////////////////////////

					for(int j=0; j< number_of_element_nodes ; j++){
						int node_no = Inverse_Node_Map[Element_Connectivity[connectivity_index+j]];
						for(int k=0; k< 18 ; k++){
							Node_Stress_And_Strain_Field[node_no][k] = Node_Stress_And_Strain_Field[node_no][k] + Stress_Strain_At_Nodes[j][k] ;
						}
					}

				}
				else{

					cout << "<<<<pvESSI>>>> Build_Stress_Field_At_Nodes:: Warning!! Gauss_Interpolation to nodes not implemented for element of Class_Tag " << Class_Tags << endl;
				}

			}
		}	

		// Now take average of contributions from gauss points 
		
		double epsilon = 1e-6;
		for(int i =0; i<Number_of_Nodes ; i++ ){
			for(int j=0; j<18; j++)
				Node_Stress_And_Strain_Field[i][j] =  Node_Stress_And_Strain_Field[i][j]/((double)Number_of_Gauss_Elements_Shared[i]+epsilon);
		}

		offset3[0] = 0;  					     offset3[1] =dims3[1]-1;    offset3[2] = 0;
	    count3 [0] = this->Number_of_Nodes;		 count3 [1] = 1;		    count3 [2] = 18;
	    dims2_out[0] =this->Number_of_Nodes;	 dims2_out[1] = 18;

	    DataSpace = H5Dget_space(id_Stress_and_Strain);
	    MemSpace = H5Screate_simple(2,dims2_out,NULL);
	    H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset3,NULL,count3,NULL);
	    H5Dwrite(id_Stress_and_Strain, H5T_NATIVE_DOUBLE, MemSpace, DataSpace, H5P_DEFAULT, Node_Stress_And_Strain_Field); 
	    H5Sclose(MemSpace); status=H5Sclose(DataSpace);


		Int_Variable_1 = dims3[1]-1;
		count1[0]   =1;
		dims1_out[0]=1;
		index_i = (hsize_t)Node_Mesh_Current_Time;

	    HDF5_Write_INT_Array_Data(id_Whether_Stress_Strain_Build,
		                          1,
		                         dims1_out,
		                         &index_i,
		                         NULL,
		                         count1,
		                         NULL,
		                         &Int_Variable_1); // Whether_Stress_Strain_Build_Index 

	}

	///////////////////// Reading the stress-strain at nodes ///////////////////////////

	offset3[0] = 0;  					     offset3[1] =dims3[1]-1;    offset3[2] = 0;
    count3 [0] = this->Number_of_Nodes;		 count3 [1] = 1;		    count3 [2] = 18;
    dims2_out[0] =this->Number_of_Nodes;	 dims2_out[1] = 18;

    DataSpace = H5Dget_space(id_Stress_and_Strain);
    MemSpace = H5Screate_simple(2,dims2_out,NULL);
    H5Sselect_hyperslab(DataSpace,H5S_SELECT_SET,offset3,NULL,count3,NULL);
    H5Dread(id_Stress_and_Strain, H5T_NATIVE_DOUBLE, MemSpace, DataSpace, H5P_DEFAULT, Node_Stress_And_Strain_Field); 
    H5Sclose(MemSpace); status=H5Sclose(DataSpace);

	for(int i=0; i< this->Number_of_Nodes; i++){	

		float El_Strain_Tuple[6] ={
			Node_Stress_And_Strain_Field[i][0],
			Node_Stress_And_Strain_Field[i][3],
			Node_Stress_And_Strain_Field[i][4],
			Node_Stress_And_Strain_Field[i][1],
			Node_Stress_And_Strain_Field[i][5],
			Node_Stress_And_Strain_Field[i][2]
		};

		float Pl_Strain_Tuple[6] ={
			Node_Stress_And_Strain_Field[i][6],
			Node_Stress_And_Strain_Field[i][9],
			Node_Stress_And_Strain_Field[i][10],
			Node_Stress_And_Strain_Field[i][7],
			Node_Stress_And_Strain_Field[i][11],
			Node_Stress_And_Strain_Field[i][8]
		};

		float Stress_Tuple[6] ={
			Node_Stress_And_Strain_Field[i][12],
			Node_Stress_And_Strain_Field[i][15],
			Node_Stress_And_Strain_Field[i][16],
			Node_Stress_And_Strain_Field[i][13],
			Node_Stress_And_Strain_Field[i][17],
			Node_Stress_And_Strain_Field[i][14]
		};

		Elastic_Strain->InsertTypedTuple (i,El_Strain_Tuple);
		Plastic_Strain->InsertTypedTuple (i,Pl_Strain_Tuple);
		Stress->InsertTypedTuple (i,Stress_Tuple);
	}

	Node_Mesh->GetPointData()->AddArray(Elastic_Strain);
	Node_Mesh->GetPointData()->AddArray(Plastic_Strain);
	Node_Mesh->GetPointData()->AddArray(Stress);

	cout << "<<<<pvESSI>>>> Build_Stress_Field_At_Nodes:: Calculation done for this step  " << Node_Mesh_Current_Time << endl;

	return;
}

