#ifndef __pvESSI_h
#define __pvESSI_h
 
#include "vtkPolyDataAlgorithm.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkQuadratureSchemeDictionaryGenerator.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationStringKey.h"
#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"
#include <map>
#include <vector>
#include <iostream>
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
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include "vtkExecutive.h"
#include <sstream>
#include <vtkPointSet.h>
#include "hdf5.h"
#include <boost/regex.hpp>
#include <vtkAppendFilter.h>
// #include <QApplication>
// #include <QStyle>
#include "vtkDataArraySelection.h"

// // #include "pqApplicationCore.h"
// #include "pqObjectBuilder.h"
// #include "pqServer.h"
// #include "pqServerManagerModel.h"
// #include "pqUndoStack.h"

class pvESSI : public vtkUnstructuredGridAlgorithm{

public:
  vtkTypeMacro(pvESSI,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  void Enable_Gauss_To_Node_Interpolation(int x){ if(!(x and Enable_Gauss_To_Node_Interpolation_Flag))this->Modified();  Enable_Gauss_To_Node_Interpolation_Flag=false; if(x) Enable_Gauss_To_Node_Interpolation_Flag=true; }
  void Show_Gauss_Mesh(int x){ if(!(x and Show_Gauss_Mesh_Flag))this->Modified(); Show_Gauss_Mesh_Flag=false; if(x) Show_Gauss_Mesh_Flag=true;}
  void Enable_uPU_Visualization(int x ){if(!(x and Enable_uPU_Visualization_Flag))this->Modified();  Enable_uPU_Visualization_Flag=false; if(x) Enable_uPU_Visualization_Flag=true;}
  void Enable_Relative_Displacement(int x ){if(!(x and Enable_Relative_Displacement_Flag))this->Modified();  Enable_Relative_Displacement_Flag=false; if(x) Enable_Relative_Displacement_Flag=true;}
  void Enable_Displacement_Probing(int x ){ if(!(x and Enable_Displacement_Probing_Flag))this->Modified();   Enable_Displacement_Probing_Flag=false; if(x) Enable_Displacement_Probing_Flag=true;}
  void Reference_Displacement_Index(int x ){if(x!=Reference_Displacement_Index_Flag)this->Modified();   Reference_Displacement_Index_Flag=x;}
  void Enable_Physical_Node_Group_Selection(int x ){ if(!(x and Enable_Physical_Node_Group_Selection_Flag))this->Modified();  Enable_Physical_Node_Group_Selection_Flag=false; if(x) Enable_Physical_Node_Group_Selection_Flag=true;}
  void Enable_Physical_Element_Group_Selection(int x ){ if(!(x and Enable_Physical_Element_Group_Selection_Flag))this->Modified();  Enable_Physical_Element_Group_Selection_Flag=false; if(x) Enable_Physical_Element_Group_Selection_Flag=true;}
  void Show_Hide_Contact(int x ){ if(!(x and Show_Hide_Contact_Flag))this->Modified();  Show_Hide_Contact_Flag=true; if(x) Show_Hide_Contact_Flag=false;}
  void Enable_Actual_Time_Step_Values(int x ){ if(!(x and Enable_Actual_Time_Step_Values_Flag))this->Modified();  Enable_Actual_Time_Step_Values_Flag=false; if(x) Enable_Actual_Time_Step_Values_Flag=true; std::cout << "Enable_Actual_Time_Step_Values " << Enable_Actual_Time_Step_Values_Flag << std::endl;}
  void Refresh_Command(){this->Modified();}

  //=======================================================================
  // User defined Physical Group generation from string from Paraview GUI
  //=======================================================================
  void User_Defined_Element_Tag_Numbers(std::string INT_ARRAY)
  {

    std::stringstream ss(INT_ARRAY); int n;

    User_Defined_Physical_Element_Group.clear();
    while(ss >> n) { User_Defined_Physical_Element_Group.push_back(n);}

    std::cout << "\n================================================================\n"; 
    std::cout << "            User Defined Physical Element Group                     "; 
    std::cout << "\n================================================================\n"; 
    for (std::vector<int>::const_iterator ele  = User_Defined_Physical_Element_Group.begin(); ele != User_Defined_Physical_Element_Group.end(); ++ele)
        std::cout << *ele << ' ';
    std::cout << std::endl;

    if(Enable_Physical_Element_Group_Selection_Flag) this->Modified();

    return;
  }


  //=======================================================================
  // User defined Physical Group generation from string from Paraview GUI
  //=======================================================================
  void User_Defined_Node_Tag_Numbers(std::string INT_ARRAY)
  {

    std::stringstream ss(INT_ARRAY); int n;

    User_Defined_Physical_Node_Group.clear();
    while(ss >> n) {User_Defined_Physical_Node_Group.push_back(n);}

    std::cout << "\n================================================================\n";
    std::cout << "               User Defined Physical Node Group                     ";
    std::cout << "\n================================================================\n";
    for (std::vector<int>::const_iterator ele = User_Defined_Physical_Node_Group.begin(); ele != User_Defined_Physical_Node_Group.end(); ++ele)
        std::cout << *ele << ' ';
    std::cout << std::endl;

    if(Enable_Physical_Node_Group_Selection_Flag) this->Modified();


    return;
  }



  void Set_Domain_Building_of_Map_Status();
  void PrintX(int x){}

  // static vtkInformationQuadratureSchemeDefinitionVectorKey* DICTIONARY();
  // static vtkInformationStringKey* QUADRATURE_OFFSET_ARRAY_NAME();

  // void GetNumberOfCellArrays();
 
  static pvESSI *New();
 
  // Description:
  // Specify file name of the .abc file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  vtkSetMacro(TimeStep, int);
  vtkGetMacro(TimeStep, int);
  int GetNumberOfTimeSteps(){
    return this->Number_of_Time_Steps;
  }

  /*************** Node Visualization Options ****************************/
  /* The user needs to select the arrays to be visualize on nodes        */
  void SetVisualizationOptionsOnNodeArrayStatus(const char* name, int status);
  const char* GetVisualizationOptionsOnNodeArrayName(int index);
  int GetNumberOfVisualizationOptionsOnNodeArrays();
  int GetVisualizationOptionsOnNodeArrayStatus(const char* name);

  /*************** Element Visualization Options ****************************/
  /* The user needs to select the arrays to be visualize on elements        */
  void SetVisualizationOptionsOnElementArrayStatus(const char* name, int status);
  const char* GetVisualizationOptionsOnElementArrayName(int index);
  int GetNumberOfVisualizationOptionsOnElementArrays();
  int GetVisualizationOptionsOnElementArrayStatus(const char* name);

  /*************** Gauss Visualization Options ****************************/
  /* The user needs to select the arrays to be visualize on gauss        */
  void SetVisualizationOptionsOnGaussArrayStatus(const char* name, int status);
  const char* GetVisualizationOptionsOnGaussArrayName(int index);
  int GetNumberOfVisualizationOptionsOnGaussArrays();
  int GetVisualizationOptionsOnGaussArrayStatus(const char* name);


  /*************** Physical Element Groups ****************************/
  /* The user needs to select the physical element group to visualize */
  void SetPhysicalElementGroupArrayStatus(const char* name, int status);
  const char* GetPhysicalElementGroupArrayName(int index);
  int GetNumberOfPhysicalElementGroupArrays();
  int GetPhysicalElementGroupArrayStatus(const char* name);

  /*************** Physical Node Groups ****************************/
  /* The user needs to select the physical node group to visualize */
  void SetPhysicalNodeGroupArrayStatus(const char* name, int status);
  const char* GetPhysicalNodeGroupArrayName(int index);
  int GetNumberOfPhysicalNodeGroupArrays();
  int GetPhysicalNodeGroupArrayStatus(const char* name);

  void INTERPOLATE_FLOAT_Time_Data_From_2_D_Dataset(hid_t datasetId, int timeIndex1, int timeIndex2, float shapeFunction1, float shapeFunction2, float** DataArray);
  void INTERPOLATE_FLOAT_Time_Data_From_3_D_Dataset(hid_t datasetId, int timeIndex1, int timeIndex2, float shapeFunction1, float shapeFunction2, float** DataArray);

  void FLOAT_Time_Data_From_2_D_Dataset(hid_t datasetId, int timeIndex, float** DataArray);
  void FLOAT_Time_Data_From_3_D_Dataset(hid_t datasetId, int timeIndex, float** DataArray);
  void INT_Time_Data_From_1_D_Dataset(hid_t datasetId, int** DataArray);
  void INT_Data_From_Scalar_Dataset(hid_t datasetId, int* DataArray);
  void FLOAT_Data_From_Scalar_Dataset(hid_t datasetId, int* DataArray);
  void FLOAT_Time_Data_From_1_D_Dataset(hid_t datasetId, float** DataArray);

  void openDatasetIds();
  void closeDatasetIds();


protected:
  pvESSI();
  ~pvESSI(){/*std::cout << "asdas " << std::endl;*/ }
 
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestInformation( vtkInformation *, vtkInformationVector **, vtkInformationVector* );
  int ProcessRequest(vtkInformation *, vtkInformationVector ** , vtkInformationVector * );
  // int RequestUpdateExtent(vtkInformation *, vtkInformationVector ** , vtkInformationVector * );;
  static void ProgressFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
  
  //////////////////////// Important Variables /////////////////////////////////////////

  int Number_of_Strain_Strain_Info;   // Total Number_of_Stress_Strain_Data_at_Nodes [currently 22]
  bool Whether_Writing_Allowed;



  /************************************** Time parameters ***************************************/
  int TimeStep;                       // 
  int Time_Step_Range[2];             // Range of Time Steps
  int Number_of_Time_Steps;           // Stores the Number of TimeSteps in the analysis
  int Number_of_Sub_Steps;            // Stores the Number of Substeps in the analysis
  double Node_Mesh_Current_Time;      // Stores the current actual time in seconds for Node Mesh
  int Gauss_Mesh_Current_Time;        // Stores the current actual time in seconds for Gauss Mesh
  int TimeIndex1, TimeIndex2;         // Stores TimeIndex1 and TimeIndex2 having current actual time
  float InterpolationFun1,InterpolationFun2; // Stores the interpolation function 1 and 2 respectively

  /********************* For Each Piece Visualization :: Element Info parameters ***************************/
  bool Whether_Piece_Data_initialized; // Whether piece data initialized
  int Total_Number_of_Elements;       // Stores the Total Elements in Visualization
  int Max_Element_Tag;                // Stores the Maximum Element Tag in Visualization
  int Total_Number_of_Gauss_Points;   // Stores the Total Number of gauss points in Visualization

  /********************* For Each Piece Visualization :: Node Info parameters  *****************************/
  int Total_Number_of_Nodes;          // Stores the Total Node in Visualization
  int Max_Node_Tag;                   // Stores the Maximum Node Tag in Visualization

  /********************************* Output Class_DESC_ENCODING Format *************************/
  int ELE_TAG_DESC_ENCODING;

  /********************************** Model Info *************************************************/
  int Number_of_Processes_Used;         // Number of Processes used
  int Process_Number;                   // Process Id or Domain Number 
  bool single_file_visualization_mode;  // Sequential or One file Visualization Mode in Parallel
  
  /*************************** For Each Piece Visualization:: More Parameters *****************************************/
  int  Build_Map_Status;                            // Whether Map is Build
  bool Enable_Initialization_Flag;                  // Whether Initialization Done
  int  piece_no;                                    // Piece no or Processor no
  int  num_of_pieces;                               // total number of pieces or processors
  bool enable_support_reactions;                    // enable support reactions
  bool eigen_mode_on;                               // enable eigen analysis mode
  bool Whether_Node_Mesh_Array_Initialized;         // Holds whether the node mesh array initialized
  bool Whether_Node_Mesh_Attributes_Initialized;    // Holds whether node mesh attributes initialized
  bool Whether_Gauss_Mesh_Array_Initialized;        // Holds whether gauss mesh array initialized
  bool Whether_Gauss_Mesh_Attributes_Initialized;   // Holds whether gauss mesh attributes initialized
  bool Whether_Node_Mesh_Stress_Attributes_Initialized;

  /******************* DataArrays for Each Domain or Process Id in  Real-ESSI Output **************/

  bool *Domain_Data_Build_Status;       // Whether Domain Data has been build
  bool *Domain_Write_Status;            // Whether Domain Data can be written 
  bool *Domain_Read_Status;             // Whether Domain Data can be read 
  bool *Domain_Node_Map_Initialized ;   // Whether Domain Node Map has been initialized
  bool *Domain_Element_Map_Initialized; // Whether Domain Element Map initialized
  bool *Domain_Basic_Info_Initialized;  // Whether Basic Info Initialized
  bool *Domain_Building_of_Map_Status;  // Whether Maps needs to be build again

  int  domain_no;                       // current Real ESSI domain_no or process Id output to be read 
  std::string Domain_FileName;          // current Real ESSI domain_no or process Id output filename

  //// 1-D Scalar for each [Domain_Number] or Real-ESSI process Id 
  //// Arrays of of dimention equal to total number of process Ids

  /*** Element Data ***/
  int Number_of_Elements;             // Actual Number of Elements in each domain
  int Pseudo_Number_of_Elements;      // Stores the Maximum Element Tag in each domain
  int Number_of_Gauss_Points;         // Stores the Number of gauss points in each domain
  int Number_of_Connectivity_Nodes;   // Stores the Number of connectivity nodes in each domain

   /*** Node Data ***/ 
  int Number_of_Nodes;                // Actual Number of Nodes in each domain
  int Pseudo_Number_of_Nodes;         // Stores the Maximum Node Tag in each domain
  int Number_of_Constrained_Dofs;     // Number of constrained dofs in each domain

  /*** Element Data ***/
  int *Domain_Number_of_Elements;             // Actual Number of Elements in each domain
  int *Domain_Pseudo_Number_of_Elements;      // Stores the Maximum Element Tag in each domain
  int *Domain_Number_of_Gauss_Points;         // Stores the Number of gauss points in each domain
  int *Domain_Number_of_Connectivity_Nodes;   // Stores the Number of connectivity nodes in each domain

   /*** Node Data ***/ 
  int *Domain_Number_of_Nodes;                // Actual Number of Nodes in each domain
  int *Domain_Pseudo_Number_of_Nodes;         // Stores the Maximum Node Tag in each domain
  int *Domain_Number_of_Constrained_Dofs;     // Number of constrained dofs in each domain


  //// 2-Dimensional Containers [Domain_Number][Array] Parameter 
  int **Domain_Class_Tags;
  int **Domain_Connectivity;
  int **Domain_Element_Map;
  int **Domain_Node_Map;
  int **Domain_Inverse_Element_Map;
  int **Domain_Inverse_Node_Map;
  int **Domain_Number_of_Dofs;
  int **Domain_Number_of_Elements_Shared;
  int **Domain_Number_of_Gauss_Elements_Shared;
  int **Domain_Number_of_Contact_Elements_Shared;  

  //// Visualization Parameters 
  bool *Whether_Node_Mesh_build;                     // Whether Node Mesh  Build for each Domain 
  bool *Whether_Gauss_Mesh_build;                    // Whether Gauss Mesh Build For each Domain 



  ///////////////////////////// HDF5 ID /////////////////////////////////////////////////////// 

  /***************** File_id **********************************/
  hid_t id_File;

  /***************** Time Steps *******************************/
  hid_t id_time;
  hid_t id_Number_of_Time_Steps;

  /***************** Model Info *******************************/
  hid_t id_Model_group; 
  hid_t id_Number_of_Elements;
  hid_t id_Number_of_Nodes;
  hid_t id_Number_of_Gauss_Points;
  hid_t id_Number_of_Processes_Used;
  hid_t id_Process_Number;

  /**************** Element Info ******************************/
  hid_t id_Elements_group;
  hid_t id_Class_Tags;
  hid_t id_Element_Class_Desc;
  hid_t id_Connectivity;
  hid_t id_Gauss_Point_Coordinates;
  hid_t id_Index_to_Connectivity;
  hid_t id_Material_Tags;
  hid_t id_Element_Outputs;
  hid_t id_Gauss_Outputs;

  /**************** Node Info ******************************/
  hid_t id_Nodes_group;
  hid_t id_Constrained_DOFs;
  hid_t id_Constrained_Nodes;
  hid_t id_Coordinates;
  hid_t id_Generalized_Displacements;
  hid_t id_Index_to_Coordinates;
  hid_t id_Support_Reactions;
  hid_t id_Number_of_DOFs;
  hid_t id_Index_to_Displacements;

  /**************** Maps ***********************************/
  hid_t id_pvESSI; 
  hid_t id_Element_Map;
  hid_t id_Node_Map;
  hid_t id_Inverse_Node_Map;
  hid_t id_Inverse_Element_Map;
  hid_t id_Number_of_Elements_Shared;
  hid_t id_Number_of_Gauss_Elements_Shared;
  hid_t id_pvESSI_Class_Tags;
  hid_t id_pvESSI_Connectivity;
  hid_t id_pvESSI_Number_of_DOFs;


  /*************** Partition Info *************************/
  hid_t id_Node_Partition;
  hid_t id_Element_Partition;

  /*************** Field at Nodes ***************************/
  hid_t id_Field_at_Nodes_group;
  hid_t id_Stress_and_Strain;
  hid_t id_Whether_Stress_Strain_Build;
  hid_t id_Energy;                            // Not implemented
  hid_t id_Whether_Energy_Build;              // Not implemented

  /************** Substep Outputs ***************************/
  hid_t id_Number_of_Iterations;
  hid_t id_Iterative_Generalized_Displacements;
  hid_t id_Iterative_Element_Outputs;
  hid_t id_Iterative_Gauss_Outputs;

  /************* Eigen Mode Analysis ***********************/
  hid_t id_Eigen_Mode_Analysis;
  hid_t id_Eigen_Frequencies;
  hid_t id_Eigen_Modes;
  hid_t id_Number_of_Eigen_Modes;
  hid_t id_Eigen_periods;
  hid_t id_Eigen_values;

  /************** Physical Groups ***************************/
  hid_t id_Physical_Element_Groups;
  hid_t id_Physical_Node_Groups;

  /************** General Variable **************************/
  hid_t DataSpace;
  hid_t DataSet;
  hid_t Group; 
  hid_t MemSpace;
  hid_t id_H5F_CLOSE_STRONG;
  hid_t id_H5F_READ_ONLY;
  hid_t id_H5F_READ_WRITE;


  /************** Visualization Control Variables **********/
  bool Enable_Gauss_To_Node_Interpolation_Flag;
  bool Enable_uPU_Visualization_Flag;
  bool Enable_Relative_Displacement_Flag;
  int  Reference_Displacement_Index_Flag;
  bool Show_Gauss_Mesh_Flag;
  bool Enable_Displacement_Probing_Flag;
  bool Whether_Physical_Group_Info_build;
  bool Enable_Physical_Element_Group_Selection_Flag;
  bool Enable_Physical_Node_Group_Selection_Flag;
  bool Show_Hide_Contact_Flag;
  bool Enable_Actual_Time_Step_Values_Flag; 

  /************* Data Array Selection *********************/  
  vtkSmartPointer<vtkDataArraySelection> Physical_Node_Group;
  vtkSmartPointer<vtkDataArraySelection> Physical_Element_Group;
  vtkSmartPointer<vtkDataArraySelection> Visualization_Options_On_Node;
  vtkSmartPointer<vtkDataArraySelection> Visualization_Options_On_Element;
  vtkSmartPointer<vtkDataArraySelection> Visualization_Options_On_Gauss;


/****************** Mesh Variables *******************/
  vtkSmartPointer<vtkPoints> points;
  
  /*********** Some HDF5 Variables Regularly Used **********/

  hsize_t  dims1_out[1], dims2_out[2], dims3_out[3]; 
  hsize_t  dims3[3],     dims2[2];
  hsize_t  maxdims3[3],  maxdims2[2];

  hsize_t  block3[3],  block2[2],  block1[1];
  hsize_t  count3[3],  count2[2],  count1[1];
  hsize_t offset3[3], offset2[2], offset1[1];
  hsize_t stride3[3], stride2[2], stride1[1];

  herr_t status;

  hsize_t index_i,index_j,index_k;


  // temporary variable store node and element number 
  int node_no, element_no, gauss_no, gauss_no_for_attributes;

  /************* Hdf5 function ******************************/


  // Read INT Array Data 
  void HDF5_Read_INT_Array_Data(hid_t id_DataSet,
                               int rank,
                               hsize_t *data_dims,
                               hsize_t *offset,
                               hsize_t *stride,
                               hsize_t *count,
                               hsize_t *block,
                               int* data);

  // Write INT Array Data 
  void HDF5_Write_INT_Array_Data(hid_t id_DataSet,
                                 int rank,
                                 hsize_t *data_dims,
                                 hsize_t *offset,
                                 hsize_t *stride,
                                 hsize_t *count,
                                 hsize_t *block,
                                 int* data);

  // To Explore the contents of HDF5 Group
  static herr_t op_func (hid_t loc_id, const char *name, const H5O_info_t *info, void *operator_data);


  /************************* Some common Variables ********************************/

  // vtkTimeStamp ReadMTime;
  // int ReadError;

// // incrementally fine-tuned progress updates.
//   virtual void GetProgressRange(float* range);
//   virtual void SetProgressRange(const float range[2], int curStep, int numSteps);
//   virtual void SetProgressRange(const float range[2], int curStep, const float* fractions);
//   virtual void UpdateProgressDiscrete(float progress);
//   float ProgressRange[2];

  vtkDataObject* GetCurrentOutput();
  vtkInformation* GetCurrentOutputInformation();
 
private:

  std::map<int,int> ESSI_to_VTK_Element;                    // ESSI to VTK Element Type Mapping
  std::map<int,std::vector<int> > ESSI_to_VTK_Connectivity; // VTK to ESSI Element Number Mapping for the Current Visualization Mode
  std::map<double,int > Time_Map; 
  std::map<std::string,int > Meta_Array_Map;
  std::map<int,double**> Gauss_To_Node_Interpolation_Map;
  int *ESSI_To_VTK_Node_Map;               // ESSI to VTK Node Number Mapping for the Current Visualization Mode
  int *ESSI_To_VTK_Element_Map;            // ESSI to VTK Element Number Mapping for the Current Visualization Mode
  int *VTK_To_ESSI_Node_Map;               // VTK to ESSI Node Number Mapping for the Current Visualization Mode
  int *VTK_To_ESSI_Element_Map;            // VTK to ESSI Element Number Mapping for the Current Visualization Mode
  int **Number_of_Elements_Shared;         // Number of Elements Shared for Each ESSI Element in Current Visualization
  int **Number_of_gauss_Elements_Shared;   // Number Gauss Elements Shared for Each ESSI Element in Current Visualization
  int **Number_of_Element_Types_Shared;    // Number Contact Elements Shared for Each ESSI Element in Current Visualization


  std::vector<int> User_Defined_Physical_Element_Group; // User Defined Physical Element Group
  std::vector<int> User_Defined_Physical_Node_Group;    // Usr Defined Physical Node Group

  pvESSI(const pvESSI&);  // Not implemented.
  void operator=(const pvESSI&);  // Not implemented.
  void set_VTK_To_ESSI_Elements_Connectivity();
  void Initialize();
  void Build_Time_Map();
  void Update_Time_Steps();
  void Domain_Initializer(int Piece_No); 
  void Initialize_Piece_data(int start, int end);
  void Close_File();
  void Build_Local_Domain_Maps(int domain_no);
  void Write_Local_Domain_Maps(int domain_no);
  void Read_Local_Domain_Maps(int domain_no);
  void Build_Meta_Array_Map();
  void Build_Gauss_To_Node_Interpolation_Map();
  void Build_Shared_Info_Per_Mode();
  void Build_Gauss_Attributes(vtkSmartPointer<vtkUnstructuredGrid> Gauss_Mesh, int Time_Index1, int Time_Index2, float Interpolation_Func1,float Interpolation_Func2);
  void Build_Node_Attributes(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Time_Index1, int Time_Index2, float Interpolation_Func1,float Interpolation_Func2);
  void Build_Eigen_Modes_Node_Attributes(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Time_Index1, int Time_Index2, float Interpolation_Func1,float Interpolation_Func2);
  void Build_Delaunay3D_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> Mesh);
  void Build_ProbeFilter_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> Probe_Input, int Time_Index1, int Time_Index2, float Interpolation_Func1,float Interpolation_Func2); // Probing variables at gauss nodes from node mesh
  void Build_Node_Stress(vtkSmartPointer<vtkUnstructuredGrid> Node_Mesh, int Time_Index1, int Time_Index2, float Interpolation_Func1,float Interpolation_Func2);
  void Interpolate_Stress_Field_At_Nodes(int Time_Index1, int timeIndex2, float Interpolation_Func1,float Interpolation_Func2, float **DataArray);
  void Build_Stress_Field_At_Nodes(int Time_Index1, float ** DataArray);
  void Extract_Stress_Field_At_Nodes(int Time_Index1, float ** DataArray);
  void Write_Stress_Field_At_Nodes(int Time_Index1, float ** DataArray);
  void Build_Physical_Element_Group_Mesh(vtkSmartPointer<vtkUnstructuredGrid> NodeMesh);
  void Build_VTK_Element_Selection_mesh();
  std::string GetSourceFile(std::string filename);



  double *Time;              // Holds the time vector for the domain
  char*  FileName;           // Holds the filename
  int*   Element_Desc_Array; // Holds the element decription array 

  /******************************************* Mesh ******************************************/  
  vtkSmartPointer<vtkUnstructuredGrid> Visualization_Current_UGrid_Node_Mesh;  // Contains the mesh of current Visualization
  vtkSmartPointer<vtkUnstructuredGrid> Visualization_Current_UGrid_Gauss_Mesh; // Contains the mesh of current Visualization
  vtkSmartPointer<vtkUnstructuredGrid> Visualization_UGrid_Node_Mesh;          // Contains the mesh of current Visualization
  vtkSmartPointer<vtkUnstructuredGrid> Visualization_UGrid_Gauss_Mesh;         // Contains the mesh of current Visualization


  /****************************************** Physical Groups*********************************/
  static std::vector<std::string> Physical_Group_Container;
  /******************************* Meta Data Arrays ********************************************/

  // Meta Data Arrays
  vtkSmartPointer<vtkFloatArray> Generalized_Displacements;
  vtkSmartPointer<vtkIntArray>   Boundary_Conditions;
  vtkSmartPointer<vtkFloatArray> Support_Reactions;
  vtkSmartPointer<vtkFloatArray> Generalized_Forces;
  vtkSmartPointer<vtkFloatArray> Generalized_Velocity;
  vtkSmartPointer<vtkFloatArray> Generalized_Acceleration;
  vtkSmartPointer<vtkFloatArray> Fluid_Displacements;
  vtkSmartPointer<vtkFloatArray> Pore_Pressure;

  // Stress-Strain 
  vtkSmartPointer<vtkFloatArray> Elastic_Strain;
  vtkSmartPointer<vtkFloatArray> Plastic_Strain;
  vtkSmartPointer<vtkFloatArray> Stress;

  // Stress-Strain Invariants
  vtkSmartPointer<vtkFloatArray> q;
  vtkSmartPointer<vtkFloatArray> p;
  vtkSmartPointer<vtkFloatArray> Plastic_Strain_q;
  vtkSmartPointer<vtkFloatArray> Plastic_Strain_p;

  // Tags
  vtkSmartPointer<vtkIntArray>   Material_Tag;
  vtkSmartPointer<vtkIntArray>   Node_Tag;
  vtkSmartPointer<vtkIntArray>   Element_Tag;
  vtkSmartPointer<vtkIntArray>   Class_Tag;

  // Energy 
  vtkSmartPointer<vtkIntArray>   Total_Energy;
  vtkSmartPointer<vtkIntArray>   Incremental_Energy;  

  // Partition Information 
  vtkSmartPointer<vtkIntArray>   Partition_Info; // For Elements


  /***************************** Mesh Building Functions ******************************************/

  void Get_Node_Mesh(vtkSmartPointer<vtkUnstructuredGrid> UGrid_Node_Mesh); 	    // Building the node mesh skeleton
  void Get_Gauss_Mesh(vtkSmartPointer<vtkUnstructuredGrid> UGrid_Gauss_Mesh); 	  // Building the gauss mesh skeleton
  void Merge_Mesh(int start, int end, vtkSmartPointer<vtkUnstructuredGrid> Mesh); // Merge domain mesh  when necessary
  void Set_Meta_Array(int Meta_Data_Id );

  void Build_Inverse_Matrices();
  void Build_Brick_Coordinates();
  double Brick_8_Gauss_Coordinates[8][3];
  double Brick_20_Gauss_Coordinates[20][3];
  double Brick_27_Gauss_Coordinates[27][3];
  double Brick_Coordinates[27][3];
  double **Twenty_Node_Brick_Inverse;
  double **Twenty_Seven_Node_Brick_Inverse;
  double **Eight_Node_Brick_Inverse;
  /*******************************************************************************************/

};
 
#endif
