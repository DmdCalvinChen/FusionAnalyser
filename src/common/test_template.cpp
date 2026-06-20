#include "ml_mesh_type.h"
int main() {
    CMeshO mesh;
    mesh.face.push_back(CFaceO());
    mesh.face[0].V(0);
    return 0;
}
