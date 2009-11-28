#/bin/sh

cd src/client

#/usr/bin/i486-mingw32-g++ -O3 -fomit-frame-pointer -march=i686 -mtune=generic -mmmx -msse -mfpmath=sse -ffast-math  -I/home/davorin/Projects/mingw32/include -I/home/davorin/Projects/mingw32/include/SDL -Wall -Wextra -Winvalid-pch -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor -Wdisabled-optimization -pipe -fno-rtti -I/home/davorin/Projects/openzone/mingw32/include -I/home/davorin/Projects/openzone/mingw32/include/SDL   -L/home/davorin/Projects/openzone/mingw32/lib -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_net -lopengl32 -lglu32 -lOpenAL32 -lalut -lvorbisfile -llua CMakeFiles/openzone.dir/Audio.cpp.obj CMakeFiles/openzone.dir/BasicAudio.cpp.obj CMakeFiles/openzone.dir/BSP.cpp.obj CMakeFiles/openzone.dir/Camera.cpp.obj CMakeFiles/openzone.dir/Colors.cpp.obj CMakeFiles/openzone.dir/Context.cpp.obj CMakeFiles/openzone.dir/Frustum.cpp.obj CMakeFiles/openzone.dir/Game.cpp.obj CMakeFiles/openzone.dir/Main.cpp.obj CMakeFiles/openzone.dir/MD2.cpp.obj CMakeFiles/openzone.dir/MD2Model.cpp.obj CMakeFiles/openzone.dir/MD2StaticModel.cpp.obj CMakeFiles/openzone.dir/MD3.cpp.obj CMakeFiles/openzone.dir/MD3StaticModel.cpp.obj CMakeFiles/openzone.dir/Model.cpp.obj CMakeFiles/openzone.dir/Network.cpp.obj CMakeFiles/openzone.dir/OBJ.cpp.obj CMakeFiles/openzone.dir/OBJModel.cpp.obj CMakeFiles/openzone.dir/OBJVehicleModel.cpp.obj CMakeFiles/openzone.dir/Render.cpp.obj CMakeFiles/openzone.dir/Shape.cpp.obj CMakeFiles/openzone.dir/Sky.cpp.obj CMakeFiles/openzone.dir/Sound.cpp.obj CMakeFiles/openzone.dir/SparkModel.cpp.obj CMakeFiles/openzone.dir/Terrain.cpp.obj CMakeFiles/openzone.dir/Water.cpp.obj  -o openzone.exe -Wl,--out-implib,libopenzone.dll.a -Wl,--major-image-version,0,--minor-image-version,0  ../ui/libui.a ../nirvana/libnirvana.a ../matrix/libmatrix.a ../base/libbase.a -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_net -lopengl32 -lglu32 ../../mingw32/lib/OpenAL32.lib -lalut -lvorbisfile -llua
/usr/bin/i486-mingw32-g++ -g3 -I/home/davorin/Projects/mingw32/include -I/home/davorin/Projects/mingw32/include/SDL -Wall -Wextra -Winvalid-pch -Wold-style-cast -Woverloaded-virtual -Wnon-virtual-dtor -Wdisabled-optimization -pipe -fno-rtti -I/home/davorin/Projects/openzone/mingw32/include -I/home/davorin/Projects/openzone/mingw32/include/SDL   -L/home/davorin/Projects/openzone/mingw32/lib -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_net -lopengl32 -lglu32 -lOpenAL32 -lalut -lvorbisfile -llua CMakeFiles/openzone.dir/Audio.cpp.obj CMakeFiles/openzone.dir/BasicAudio.cpp.obj CMakeFiles/openzone.dir/BSP.cpp.obj CMakeFiles/openzone.dir/Camera.cpp.obj CMakeFiles/openzone.dir/Colors.cpp.obj CMakeFiles/openzone.dir/Context.cpp.obj CMakeFiles/openzone.dir/Frustum.cpp.obj CMakeFiles/openzone.dir/Game.cpp.obj CMakeFiles/openzone.dir/Main.cpp.obj CMakeFiles/openzone.dir/MD2.cpp.obj CMakeFiles/openzone.dir/MD2Model.cpp.obj CMakeFiles/openzone.dir/MD2StaticModel.cpp.obj CMakeFiles/openzone.dir/MD3.cpp.obj CMakeFiles/openzone.dir/MD3StaticModel.cpp.obj CMakeFiles/openzone.dir/Model.cpp.obj CMakeFiles/openzone.dir/Network.cpp.obj CMakeFiles/openzone.dir/OBJ.cpp.obj CMakeFiles/openzone.dir/OBJModel.cpp.obj CMakeFiles/openzone.dir/OBJVehicleModel.cpp.obj CMakeFiles/openzone.dir/Render.cpp.obj CMakeFiles/openzone.dir/Shape.cpp.obj CMakeFiles/openzone.dir/Sky.cpp.obj CMakeFiles/openzone.dir/Sound.cpp.obj CMakeFiles/openzone.dir/SparkModel.cpp.obj CMakeFiles/openzone.dir/Terrain.cpp.obj CMakeFiles/openzone.dir/Water.cpp.obj  -o openzone.exe -Wl,--out-implib,libopenzone.dll.a -Wl,--major-image-version,0,--minor-image-version,0  ../ui/libui.a ../nirvana/libnirvana.a ../matrix/libmatrix.a ../base/libbase.a -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_net -lopengl32 -lglu32 ../../mingw32/lib/OpenAL32.lib -lalut -lvorbisfile -llua

cd ../..
mv src/client/openzone.exe mingw32/bin/openzone.exe
