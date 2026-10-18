# Converts wavefront obj files into c/c++ arrays

import sys
obj_file = open(sys.argv[1])
obj_lines = obj_file.readlines()
obj_file.close()

obj_name = ""
obj_vertices = []
obj_texture_coords = []
obj_normals = []
obj_faces = []

def SaveObjArrays():
    print("Saving object: " + obj_name)

    vertex_count = 0
    vertex_size = 8
    vertices = []
    indices = []

    for face in obj_faces:

        for element in obj_vertices[int(face[0]) - 1]:
            vertices.append(float(element))
        for element in obj_normals[int(face[2]) - 1]:
            vertices.append(float(element))
        for element in obj_texture_coords[int(face[1]) - 1]:
            vertices.append(float(element))
        
        indices.append(vertex_count)
        vertex_count += 1

    print("Vertices:")
    for vertex in range(vertex_count):
        index = vertex * vertex_size
        print("Position: ", end = "")
        for position in range(index, index + 3):
            print(str(vertices[position]) + ",", end = "")
        print("\nNormal: ", end = "")
        for normal in range(index + 3, index + 6):
            print(str(vertices[normal]) + ",", end = "")
        print("\nTexture Coord: ", end = "")
        for texture_coord in range(index + 6, index + 8):
            print(str(vertices[texture_coord]) + ",", end = "")
        print("\n")
    print("Indices:")
    count = 0
    for index in indices:
        count += 1
        print(str(index) + ", ", end = "")
        if count == 3:
            print()
            count = 0

    file = open(obj_name + ".h", "w")
    file.write("const float c_Vertices[] = {\n\t")
    count = 0
    for element in vertices:
        count += 1
        if element == 0: 
            file.write("0.0f, ")
        else:
            file.write(str(element) + "f, ")
        if count == vertex_size:
            file.write("\n\t")
            count = 0
    file.write("};\n")
    
    file.write("const uint32_t c_Indices[] = {\n\t")
    count = 0
    for element in indices:
        count += 1
        file.write(str(element) + ", ")
        if count == 3:
            file.write("\n\t")
            count = 0
    file.write("};\n")
    file.close()

    obj_vertices.clear()
    obj_texture_coords.clear()
    obj_normals.clear()
    obj_faces.clear()

for line in obj_lines:
    elements = line.split(" ")
    if len(elements) == 0: continue
    line_type = elements[0]

    # remove end line
    elements[len(elements) - 1] = elements[len(elements) - 1][:-1]

    if line_type == "o":
        if obj_name != "": SaveObjArrays()
        obj_name = line[2:-1]
    elif line_type == "v":
        obj_vertices.append(elements[1:])
    elif line_type == "vn":
        obj_normals.append(elements[1:])
    elif line_type == "vt":
        obj_texture_coords.append(elements[1:])
    elif line_type == "f":
        for face in elements[1:]:
            obj_faces.append(face.split("/"))
    else: continue

SaveObjArrays()