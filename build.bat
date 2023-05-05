if not exist buildVS mkdir buildVS
cd buildVS

cmake -G "Visual Studio 17 2022" ..

cmake --build . --config Debug

pause
