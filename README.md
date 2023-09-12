# DirectX 12 Rendering

###
Playground for learning graphics programming.<br/>
Most of implemented solutions are either experimental or temporal.<br/>
<br/>
## Techniques: ##
- Deferred and Forward Rendering
- PBR (Epic's UE4 Model) with sky reflections
- Image Based Lighting
- Simple MipMapping via DirectXTex
- Directional Shadow Mapping

### Built with: ###
<ul>
<li> C++ 20 </li>
<li> DirectX 12 </li>
<li> Visual Studio 2022: MSVC, Windows SDK </li>
<li> vcpkg - manifest mode </li>
</ul>

### Third-party: ###
- [assimp](https://github.com/assimp/assimp)
- [imgui](https://github.com/ocornut/imgui)
- [spdlog](https://github.com/gabime/spdlog)
- [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator)
- [DirectXTex](https://github.com/microsoft/DirectXTex)
- [DirectXTK12](https://github.com/microsoft/DirectXTK12) -> WIC/DDS loaders

PBR + IBL
![Screenshot](screenshots/deferred_sponza_ibl.png)
![Screenshot](screenshots/deferred_helmet_ibl.png)
Directional Shadow Map
![Screenshot](screenshots/deferred_shadow_map.png)
