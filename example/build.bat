@set trunk_dir=%cd%
pushd %trunk_dir%

md %trunk_dir%\build
cd %trunk_dir%\build
cmake -G "Visual Studio 15 2017 Win64" %trunk_dir%

popd