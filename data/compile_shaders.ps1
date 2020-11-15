# Vertex 
Get-ChildItem -Filter *.vert -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}

# Tessellation Control 
Get-ChildItem -Filter *.tesc -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}

# Tessellation Evaluation
Get-ChildItem -Filter *.tese -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}

# Geometry 
Get-ChildItem -Filter *.geom -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}

# Fragment 
Get-ChildItem -Filter *.frag -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}

# Compute 
Get-ChildItem -Filter *.comp -Recurse | ForEach-object {
    $ShaderPath = $_ | Resolve-Path -Relative
    glslangValidator -V "$ShaderPath" -o "$ShaderPath.spv"
}