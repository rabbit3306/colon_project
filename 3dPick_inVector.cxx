/*

store the seed index in a vector


parameters: 
1. file you want to open

*/



#include <cstdlib>
#include <iostream>

#include <vtkObjectFactory.h>
#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkCommand.h"
#include "vtkMetaImageReader.h"
#include "vtkImageData.h"
#include "vtkImageResliceMapper.h"
#include "vtkImageSlice.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCellPicker.h"
#include "vector"

// 2DVector for seedIndex

std::vector< std::vector<int> > seedIndex;

typedef vtkSmartPointer< vtkCellPicker > PickerType;

//
//  Picking
//
class PrintPickCallback : public vtkCommand
{
public:

  static PrintPickCallback *New();
  vtkTypeMacro( PrintPickCallback, vtkCommand );

  virtual void Execute(vtkObject *, unsigned long event, void *)
    {
    int idx[3];
    std::vector<int> seed;
    picker->GetPointIJK( idx );
    std::cout << idx[0] << " " << idx[1] << " " << idx[2] << std::endl;
    for(int i = 0; i <3; i++)
        seed.push_back(idx[i]);
    seedIndex.push_back(seed);
    
    /*
    //test
    std::vector< std::vector<int> > ::iterator it;
    
    for (it = seedIndex.begin(); it != seedIndex.end(); it++)
        {
            std::vector<int> allseed;
            allseed = *it;
            std::cout << allseed[0] << " " << allseed[1] << " " << allseed[2] << std::endl;
        }
       */
     }
     

  void SetPicker( PickerType p ) { picker = p; }

private:
  PickerType picker;
};
vtkStandardNewMacro( PrintPickCallback );


//
// Advancing interactor
//
class AdvancingInteractorStyle : public vtkInteractorStyleImage
{
public:
  static AdvancingInteractorStyle * New();
  vtkTypeMacro( AdvancingInteractorStyle, vtkInteractorStyleImage );

  void AdvanceSlice( double step )
    {
    vtkRenderWindowInteractor *rwi = this->Interactor;
    double fp[3];
    double normal[3];
    vtkRenderer * renderer = this->GetCurrentRenderer();
    vtkCamera * camera = renderer->GetActiveCamera();
    camera->GetFocalPoint( fp );
    camera->GetViewPlaneNormal( normal );
    fp[0] += normal[0] * step;
    fp[1] += normal[1] * step;
    fp[2] += normal[2] * step;
    camera->SetFocalPoint( fp );
    rwi->Render();
    }

  virtual void OnChar()
    {
    // Get the keypress
    vtkRenderWindowInteractor *rwi = this->Interactor;
    char key = rwi->GetKeyCode();
    
    if ( key == 'c' )
    {
       //if made a mistake, then click 'c' to cancle last seed
       seedIndex.pop_back(); 
       /*
       //test
        std::vector< std::vector<int> > ::iterator it;
    
        for (it = seedIndex.begin(); it != seedIndex.end(); it++)
            {
                std::vector<int> allseed;
                allseed = *it;
                std::cout << allseed[0] << " " << allseed[1] << " " << allseed[2] << std::endl;
            }
        */
     }
    
    if ( key == '\'' )
      {
      //std::cout << "up" << std::endl;
      AdvanceSlice( 1.0 );
      }
    if ( key == ';' )
      {
      //std::cout << "down" << std::endl;
      AdvanceSlice( -1.0 );
      }
    // Forward events
    vtkInteractorStyleImage::OnChar();
    }
};
vtkStandardNewMacro( AdvancingInteractorStyle );

int main( int argc, char ** argv )
{
  char * in_file = argv[1];

  typedef vtkSmartPointer< vtkMetaImageReader > ReaderType;
  ReaderType reader = ReaderType::New();
  reader->SetFileName( in_file );
  reader->Update();

  typedef vtkSmartPointer< vtkImageResliceMapper > MapperType;
  MapperType mapper = MapperType::New();
  mapper->SetInputConnection( reader->GetOutputPort() );
  mapper->SliceAtFocalPointOn();
  mapper->SliceFacesCameraOn();

  typedef vtkSmartPointer< vtkImageSlice > ActorType;  
  ActorType actor = ActorType::New();
  actor->SetMapper( mapper );

  typedef vtkSmartPointer< vtkRenderer > RenderereType;
  RenderereType renderer = RenderereType::New();
  renderer->SetBackground( 0.2, 0.2, 0.2 );  
  renderer->AddActor(  actor );

  typedef vtkSmartPointer< vtkRenderWindow > WindowType;
  WindowType window = WindowType::New();
  window->AddRenderer( renderer );

  typedef vtkSmartPointer< AdvancingInteractorStyle > StyleType;
  StyleType style = StyleType::New();
  style->SetInteractionModeToImage3D();
  style->SetCurrentRenderer( renderer );

  typedef vtkSmartPointer< PrintPickCallback > PrintType;
  PrintType print_pick = PrintType::New();

  PickerType picker = PickerType::New();
  picker->AddObserver( vtkCommand::EndPickEvent, print_pick );

  print_pick->SetPicker( picker ); 

  typedef vtkSmartPointer< vtkRenderWindowInteractor > InteractorType;
  InteractorType interactor = InteractorType::New();
  interactor->SetInteractorStyle( style );
  interactor->SetPicker( picker );
  
  
  interactor->SetRenderWindow( window );
  interactor->Initialize();

  window->SetInteractor(interactor);
  window->Render();
  
  interactor->Start();

  return EXIT_SUCCESS;
}
