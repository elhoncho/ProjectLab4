# ProjectLab4
Clone the Repo (Need to do it recursivly to pull all of the submodules)
    
    git clone --recursive https://github.com/elhoncho/ProjectLab4.git

1.Setup Instructions to run Flight Controller

    This code was developed using OpenSTM32 and CubeMX
    
2.Install openstm32 IDE

    Windows:
    http://www.ac6-tools.com/downloads/SW4STM32/install_sw4stm32_win_64bits-latest.exe
    
    Mac:
    http://www.ac6-tools.com/downloads/SW4STM32/install_sw4stm32_macos_64bits-latest.run
        Set to executable 
            chmod 777 install_sw4stm32_macos_64bits-latest.run
        then run
            ./install_sw4stm32_macos_64bits-latest.run

3.Install cubemx eclipse plugin

    https://my.st.com/content/my_st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stsw-stm32095.html
    (Document UM1718 for instructions)

4.Open the cubemx perspective in Eclipse

5.Install f4 plugin 

    (4.5.2 of UM1718)

6.Install git

    Not necessary if ruing Linux or Mac

7.Clone Repo

    git clone --recurse-submodules https://github.com/elhoncho/ProjectLab4.git

8.In eclipse file->open new project from file system

9.Build the project

10.Debug button -> debug configuration

    select Ac6 STM32
    
11. Add AT86RF212B/Inc to the includes path in Eclipse

12. Add AT85RF212B/Src to the sources path in Eclipse
