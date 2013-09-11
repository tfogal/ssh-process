#include <cstdlib>
#include <iostream>
#include <vtkImageData.h>
#include <vtkMarchingCubes.h>
#include <vtkPNrrdReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

#if VTK_MAJOR_VERSION != 5
# warning "vtk5 required.  (tested on 5.8)"
#endif

int main(int argc, char *argv[])
{
  if(argc != 3) {
    std::cerr << "Usage: " << argv[0] << " nhdr isovalue\n";
    return EXIT_FAILURE;
  }
  vtkSmartPointer<vtkImageData> volume = vtkSmartPointer<vtkImageData>::New();
  // we use the 'parallel' version here because our distro's VTK doesn't ship
  // with the normal/serial version (wtf?)
  {
	vtkSmartPointer<vtkPNrrdReader> rdr = vtkSmartPointer<vtkPNrrdReader>::New();
  rdr->SetFileName(argv[1]);
  rdr->Update();
	volume->DeepCopy(rdr->GetOutput());
  }

  double isoValue = atof(argv[2]);

  vtkSmartPointer<vtkMarchingCubes> surface =
    vtkSmartPointer<vtkMarchingCubes>::New();

  surface->SetInput(volume);
  surface->ComputeNormalsOn();
  surface->SetValue(0, isoValue);

  vtkSmartPointer<vtkPolyDataWriter> pdw =
    vtkSmartPointer<vtkPolyDataWriter>::New();
  pdw->SetInput(surface->GetOutput());
  pdw->SetFileName("mag.vtk");
  pdw->Update();

  return EXIT_SUCCESS;
}
