#!/bin/env sh

# Vertex
for FILE in $(find -name "*.vert"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done

# Tessellation Control
for FILE in $(find -name "*.tesc"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done

# Tessellation Evaluation
for FILE in $(find -name "*.tese"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done

# Geometry
for FILE in $(find -name "*.geom"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done

# Fragment
for FILE in $(find -name "*.frag"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done

# Compute
for FILE in $(find -name "*.comp"); do
    glslangValidator -V "$FILE" -o "$FILE.spv"
done