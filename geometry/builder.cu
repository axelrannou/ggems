// This file is part of GGEMS
//
// GGEMS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GGEMS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GGEMS.  If not, see <http://www.gnu.org/licenses/>.
//
// GGEMS Copyright (C) 2013-2014 Julien Bert

#ifndef BUILDER_CU
#define BUILDER_CU

#include "builder.cuh"

///////// BVH class ////////////////////////////////////////////////////

BVH::BVH() {}

// Return the current node id
unsigned int BVH::get_current_id() {
    return cur_node_id;
}

// Update the tree address
void BVH::update_address() {
    ptr_nodes[0] = 0;
    unsigned int i=1;
    while (i < ptr_nodes.size()) {
        ptr_nodes[i] = ptr_nodes[i-1] + size_of_nodes[i-1];
        ++i;
    }
}

// Add the root
void BVH::add_root() {
    ptr_nodes.push_back(0);
    size_of_nodes.push_back(0);
    mother_node.push_back(0);
    cur_node_id = 0;
}

// Add a node
void BVH::add_node(unsigned int mother_id) {
    // New node ID
    cur_node_id++;

    // Insert this object into the tree
    child_nodes.insert(child_nodes.begin() + ptr_nodes[mother_id] + size_of_nodes[mother_id],
                       cur_node_id);

    // Update the tree
    size_of_nodes[mother_id]++;
    size_of_nodes.push_back(0);
    ptr_nodes.push_back(cur_node_id);
    mother_node.push_back(mother_id);

    // Update tree address
    update_address();
}

// Print the BVH
void BVH::print() {
    // print each node
    unsigned int i = 0;
    unsigned int j = 0;
    while (i < size_of_nodes.size()) {
        printf("(%i)--[%i]--(", mother_node[i], i);
        j=0; while (j < size_of_nodes[i]) {
            printf("%i,", child_nodes[ptr_nodes[i]+j]);
            ++j;
        }
        printf(")\n");
        ++i;
    }
    printf("\n");
}

///////// BVH class ////////////////////////////////////////////////////

GeometryBuilder::GeometryBuilder() {}

// Print the current world
void GeometryBuilder::print() {
    // Print out the tree structure
    World.tree.print();

    // Print out every object name
    unsigned int i;
    printf("List of object:\n");
    i=0; while (i < World.name_objects.size()) {
        printf("%i - %s\n", i, World.name_objects[i].c_str());
        ++i;
    }
    printf("\n");

    // Print out every material name
    printf("List of material:\n");
    i=0; while (i < World.materials_list.size()) {
        printf("%i - %s\n", i, World.materials_list[i].c_str());
        ++i;
    }
    printf("\n");

    // Print out each object contains on the tree
    i=0; while (i < World.ptr_objects.size()) {
        // Get obj address
        unsigned int address_obj = World.ptr_objects[i];

        // Object name
        printf("::: %s :::\n", World.name_objects[i].c_str());

        // Object type
        unsigned int params1 = (unsigned int)(World.data_objects[address_obj]);
        unsigned int params2 = (unsigned int)(World.data_objects[address_obj+1]);
        switch (params1) {
        case AABB:
            printf("type: AABB\n");
            printf("material: %s\n", World.materials_list[params2].c_str());
            printf("xmin: %f xmax: %f ymin: %f ymax: %f zmin: %f zmax: %f\n\n",
                    World.data_objects[address_obj+2], World.data_objects[address_obj+3],
                    World.data_objects[address_obj+4], World.data_objects[address_obj+5],
                    World.data_objects[address_obj+6], World.data_objects[address_obj+7]);
            break;

        case SPHERE:
            printf("type: SPHERE\n");
            printf("material: %s\n", World.materials_list[params2].c_str());
            printf("cx: %f cy: %f cz: %f radius: %f\n\n",
                    World.data_objects[address_obj+2], World.data_objects[address_obj+3],
                    World.data_objects[address_obj+4], World.data_objects[address_obj+5]);
            break;

        case MESHED:
            printf("type: MESHED\n");
            printf("material: %s\n", World.materials_list[params2].c_str());
            printf("Number of triangles: %i\n", (unsigned int)World.data_objects[address_obj+2]);
            printf("xmin: %f xmax: %f ymin: %f ymax: %f zmin: %f zmax: %f\n\n",
                    World.data_objects[address_obj+3], World.data_objects[address_obj+4],
                    World.data_objects[address_obj+5], World.data_objects[address_obj+6],
                    World.data_objects[address_obj+7], World.data_objects[address_obj+8]);

            if (World.data_objects[address_obj+9] == NO_OCTREE) {
                printf("Octree: None\n");
            } else if (World.data_objects[address_obj+9] == REG_OCTREE) {
                printf("Octree: Regular (%i x %i x %i)\n",
                       World.data_objects[address_obj+10], World.data_objects[address_obj+11],
                       World.data_objects[address_obj+12]);

            }

            break;

        } // switch

        ++i;
    } // while
}

// Print out the geometry raw data
void GeometryBuilder::print_raw() {

    // Print out every object name
    unsigned int i;
    printf("List of object [%lu]: ", World.name_objects.size());
    i=0; while (i < World.name_objects.size()) {
        printf("%s ", World.name_objects[i].c_str());
        ++i;
    }
    printf("\n\n");

    // Print out every material name
    printf("List of material [%lu]: ", World.materials_list.size());
    i=0; while (i < World.materials_list.size()) {
        printf("%s ", World.materials_list[i].c_str());
        ++i;
    }
    printf("\n\n");

    // Print out size of objects
    printf("Size of objects [%lu]: ", World.size_of_objects.size());
    i=0; while (i < World.size_of_objects.size()) {
        printf("%i ", World.size_of_objects[i]);
        ++i;
    }
    printf("\n\n");

    // Print out object addresses
    printf("Object addresses [%lu]: ", World.ptr_objects.size());
    i=0; while (i < World.ptr_objects.size()) {
        printf("%i ", World.ptr_objects[i]);
        ++i;
    }
    printf("\n\n");

    // Print out object data
    printf("Object data [%lu]: ", World.data_objects.size());
    i=0; while (i < World.data_objects.size()) {
        printf("%f ", World.data_objects[i]);
        ++i;
    }
    printf("\n\n");

}

// Save the world in order to share an use it later
void GeometryBuilder::save_world(std::string filename) {

    FILE *pfile = fopen(filename.c_str(), "wb");
    unsigned int i, nb;

    // .: Tree :.  -  First export the tree that structure the world

    // 1. ptr_nodes [N, data]
    nb = World.tree.ptr_nodes.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.tree.ptr_nodes.data(), nb, sizeof(unsigned int), pfile);

    // 2. size_of_nodes [N, data]
    nb = World.tree.size_of_nodes.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.tree.size_of_nodes.data(), nb, sizeof(unsigned int), pfile);

    // 3. child_nodes [N, data]
    nb = World.tree.child_nodes.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.tree.child_nodes.data(), nb, sizeof(unsigned int), pfile);

    // 4. mother_node [N, data]
    nb = World.tree.mother_node.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.tree.mother_node.data(), nb, sizeof(unsigned int), pfile);

    // 5. cur_node_id [val]
    fwrite(&World.tree.cur_node_id, 1, sizeof(unsigned int), pfile);

    // .: World :.  -  Then export the world

    // 6. name_objects [N, data]
    nb = World.name_objects.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    i=0; while (i < nb) {
        fwrite(World.name_objects[i].c_str(), World.name_objects[i].size(), sizeof(char), pfile);
        ++i;
    }

    // 7. materials_list [N, data]
    nb = World.materials_list.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    i=0; while (i < nb) {
        fwrite(World.materials_list[i].c_str(), World.materials_list[i].size(), sizeof(char), pfile);
        ++i;
    }

    // 8. ptr_objects [N, data]
    nb = World.ptr_objects.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.ptr_objects.data(), nb, sizeof(unsigned int), pfile);

    // 9. size_of_objects [N, data]
    nb = World.size_of_objects.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.size_of_objects.data(), nb, sizeof(unsigned int), pfile);

    // 10. data_objects [N, data] (the big one!!!)
    nb = World.data_objects.size();
    fwrite(&nb, 1, sizeof(unsigned int), pfile);
    fwrite(World.data_objects.data(), nb, sizeof(float), pfile);


    fclose(pfile);


}

// Search and return the material index for a given material name
unsigned int GeometryBuilder::get_material_index(std::string material_name) {

    // Check if this material is already used, if it is return the corresponding index
    unsigned int index = 0;
    while (index < World.materials_list.size()) {
        if (World.materials_list[index] == material_name) return index;
        ++index;
    }

    // If it is not, add a new entry into the material table
    index = World.materials_list.size();
    World.materials_list.push_back(material_name);

    return index;
}

// Add the world
unsigned int GeometryBuilder::add_world(Aabb obj) {

    // Add the root tree
    World.tree.add_root();

    // Store the address to access to this object
    World.ptr_objects.push_back(World.data_objects.size());

    // Store the information of this object
    World.data_objects.push_back(AABB);                                  // Object Type
    World.data_objects.push_back(get_material_index(obj.material_name)); // Material index
    World.data_objects.push_back(obj.xmin);                              // AABB parameters
    World.data_objects.push_back(obj.xmax);
    World.data_objects.push_back(obj.ymin);
    World.data_objects.push_back(obj.ymax);
    World.data_objects.push_back(obj.zmin);
    World.data_objects.push_back(obj.zmax);

    World.name_objects.push_back(obj.object_name);                       // Name of this object

    // Store the size of this object
    World.size_of_objects.push_back(8);

    return World.tree.get_current_id();

}

// Add an AABB object into the world
unsigned int GeometryBuilder::add_object(Aabb obj, unsigned int mother_id) {

    // Add this object to the tree
    World.tree.add_node(mother_id);

    // Store the address to access to this object
    World.ptr_objects.push_back(World.data_objects.size());

    // Store the information of this object
    World.data_objects.push_back(AABB);                                  // Object Type
    World.data_objects.push_back(get_material_index(obj.material_name)); // Material index
    World.data_objects.push_back(obj.xmin);                              // AABB parameters
    World.data_objects.push_back(obj.xmax);
    World.data_objects.push_back(obj.ymin);
    World.data_objects.push_back(obj.ymax);
    World.data_objects.push_back(obj.zmin);
    World.data_objects.push_back(obj.zmax);

    World.name_objects.push_back(obj.object_name);                       // Name of this object

    // Store the size of this object
    World.size_of_objects.push_back(8);

    return World.tree.get_current_id();

}

// Add a Sphere object into the world
unsigned int GeometryBuilder::add_object(Sphere obj, unsigned int mother_id) {

    // Add this object to the tree
    World.tree.add_node(mother_id);

    // Store the address to access to this object
    World.ptr_objects.push_back(World.data_objects.size());

    // Store the information of this object
    World.data_objects.push_back(SPHERE);                                // Object Type
    World.data_objects.push_back(get_material_index(obj.material_name)); // Material index
    World.data_objects.push_back(obj.cx);                                // Sphere parameters
    World.data_objects.push_back(obj.cy);
    World.data_objects.push_back(obj.cz);
    World.data_objects.push_back(obj.radius);

    World.name_objects.push_back(obj.object_name);                       // Name of this object
    
    // Store the size of this object
    World.size_of_objects.push_back(6);

    return World.tree.get_current_id();

}

// Add a Meshed object into the world
unsigned int GeometryBuilder::add_object(Meshed obj, unsigned int mother_id) {

    // Add this object to the tree
    World.tree.add_node(mother_id);

    // Store the address to access to this object
    World.ptr_objects.push_back(World.data_objects.size());

    // Store the information of this object
    World.data_objects.push_back(MESHED);                                // Object Type

    World.data_objects.push_back(get_material_index(obj.material_name)); // Material index
    World.data_objects.push_back(obj.number_of_triangles);               // Number of triangles

    // Add the boudning box of this mesh
    World.data_objects.push_back(obj.xmin);
    World.data_objects.push_back(obj.xmax);
    World.data_objects.push_back(obj.ymin);
    World.data_objects.push_back(obj.ymax);
    World.data_objects.push_back(obj.zmin);
    World.data_objects.push_back(obj.zmax);

    // Append octree information
    World.data_objects.push_back(obj.octree_type); // NO_OCTREE, REG_OCTREE, ADP_OCTREE
    if (obj.octree_type == REG_OCTREE) {
        World.data_objects.push_back(obj.nb_cell_x); // Octree size in cells
        World.data_objects.push_back(obj.nb_cell_y);
        World.data_objects.push_back(obj.nb_cell_z);
    }

    // Append every triangle
    World.data_objects.reserve(World.data_objects.size() + obj.vertices.size());
    World.data_objects.insert(World.data_objects.end(), obj.vertices.begin(), obj.vertices.end());

    // Append the octree if defined
    if (obj.octree_type == REG_OCTREE) {
        // Append the number of objects per cell
        World.data_objects.reserve(World.data_objects.size() + obj.nb_objs_per_cell.size());
        World.data_objects.insert(World.data_objects.end(), obj.nb_objs_per_cell.begin(),
                                                            obj.nb_objs_per_cell.end());
        // Append the addr of each cell
        World.data_objects.reserve(World.data_objects.size() + obj.addr_to_cell.size());
        World.data_objects.insert(World.data_objects.end(), obj.addr_to_cell.begin(),
                                                            obj.addr_to_cell.end());
        // Append the list of objects per cell
        World.data_objects.reserve(World.data_objects.size() + obj.list_objs_per_cell.size());
        World.data_objects.insert(World.data_objects.end(), obj.list_objs_per_cell.begin(),
                                                            obj.list_objs_per_cell.end());
    }

    // Name of this object
    World.name_objects.push_back(obj.object_name);

    // Store the size of this object
    if (obj.octree_type == REG_OCTREE) {
        World.size_of_objects.push_back(obj.vertices.size() + obj.nb_objs_per_cell.size() +
                                        obj.addr_to_cell.size() + obj.list_objs_per_cell.size() + 13);
    } else { // NO_OCTREE
        World.size_of_objects.push_back(obj.vertices.size()+10);
    }

    return World.tree.get_current_id();

}

// Add a Meshed object into the world
unsigned int GeometryBuilder::add_object(Voxelized obj, unsigned int mother_id) {

    // Add this object to the tree
    World.tree.add_node(mother_id);

    // Store the address to access to this object
    World.ptr_objects.push_back(World.data_objects.size());

    // Store the information of this object
    World.data_objects.push_back(VOXELIZED);                                // Object Type

    // Store the parameters of this object
    World.data_objects.push_back(obj.number_of_voxels);
    World.data_objects.push_back(obj.nb_vox_x);
    World.data_objects.push_back(obj.nb_vox_y);
    World.data_objects.push_back(obj.nb_vox_z);
    World.data_objects.push_back(obj.spacing_x);
    World.data_objects.push_back(obj.spacing_y);
    World.data_objects.push_back(obj.spacing_z);

    // Add the boudning box of this phantom
    World.data_objects.push_back(obj.xmin);
    World.data_objects.push_back(obj.xmax);
    World.data_objects.push_back(obj.ymin);
    World.data_objects.push_back(obj.ymax);
    World.data_objects.push_back(obj.zmin);
    World.data_objects.push_back(obj.zmax);

    // Here we need to merge and update the material ID according the current list of mats

    // Build a LUT to convert the old id in new one
    std::vector<unsigned int> new_id;
    unsigned int i = 0;
    while (i < obj.list_of_materials.size()) {
        new_id.push_back(get_material_index(obj.list_of_materials[i]));
        ++i;
    }

    // Now convert every material ID contains on the voxelized volume
    i=0; while (i < obj.number_of_voxels) {
        obj.data[i] = new_id[obj.data[i]];
        ++i;
    }

    // Finally append voxelized data into the world
    World.data_objects.reserve(World.data_objects.size() + obj.data.size());
    World.data_objects.insert(World.data_objects.end(), obj.data.begin(), obj.data.end());

    // Name of this object
    World.name_objects.push_back(obj.object_name);

    // Store the size of this object
    World.size_of_objects.push_back(obj.data.size()+14);

    return World.tree.get_current_id();

}

#endif
