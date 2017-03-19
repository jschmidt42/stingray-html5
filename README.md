# Autodesk® Stingray / HTML5 plugin

Render HTML5 views in Stingray. The engine plugin uses [CEF](https://bitbucket.org/chromiumembedded/cef) to render HTML5 content to texture buffers. Once the texture buffers are generated, the render resource is assigned to the view's material `html5_texture` slot.

![image](https://cloud.githubusercontent.com/assets/4054655/23341516/1062bbda-fc17-11e6-807f-11dd64628d52.png)

## Features

- Script the engine application entirely using JavaScript (see `plugin/sample_project/app.html5/app.js`).
- Expose Stingray API to JavaScript.
- Render 2D web views fullscreen to an engine window.
- Render 3D web views on level object meshes.
- Compile `.html5` folder as accessible HTML content at runtime using the `stingray://` scheme.

## Install prerequisites

-   Git client: <https://git-scm.com/>
-   Ruby 2.0 or later: <http://rubyinstaller.org>.
    -   Rubygems SSL fix (if needed): <http://guides.rubygems.org/ssl-certificate-update>
-   Visual Studio 2015 with Update 3 & Patch KB3165756: <https://www.visualstudio.com/downloads/#visual-studio-professional-2015-with-update-3>

## Build

> ./make.rb

## Install plugin

1. Start editor
2. Open plugin manager (**Alt+Shift+P**) and select `plugin/html5.plugin`.

    ![image](https://cloud.githubusercontent.com/assets/4054655/23341629/287962ee-fc19-11e6-8256-36c185993d0b.png)

3. Create new HTML Sample project (e.g. from the HTML5 Sample under the Templates tab in the project manager)

    ![image](https://cloud.githubusercontent.com/assets/4054655/23341632/3bab7816-fc19-11e6-87d6-53b4cc3936de.png)

## Run sample project

Once the sample project is the selected project in your tool chain configuration you can compile and run it using:

`$SR_BIN_DIR`: The binary directory where Stingray is installed.

> `stingray_win64_dev.exe --toolchain "$SR_BIN_DIR" --plugin-dir "./plugin/binaries/engine/win64/dev"`

## Compile and run project

> stingray_win64_dev.exe --source-dir "./plugin/sample_project" --data-dir "./build/sample_project_data/win32" --map-source-dir core "$SR_BIN_DIR" --map-source-dir html5_resources "./plugin" --plugin-dir "./plugin/binaries/engine/win64/dev" --compile --continue --silent-mode

If everything worked well, you should see the engine booting up and displaying some web views.

## Debug the editor engine instance

1. Start the editor engine with the following options:

    > stingray_win64_dev.exe --data-dir "G:\stingray-html5\build\sample_project_data\win32" --viewport-provider --disable-default-window --editor-ini "core/editor_slave/stingray_editor/settings" --no-raw-input

2. Then start the Stingray editor. The editor will attach to the instance you started manually if the command line matches. It is important that you specify the same command line arguments than the editor would launch this instance. You can use a tool like Process Explorer to look for these options.

## Screenshots

#### 3D web views

![image](https://cloud.githubusercontent.com/assets/4054655/23341454/023683da-fc16-11e6-9785-f7e64f8d0682.png)

![image](https://cloud.githubusercontent.com/assets/4054655/23341459/144ed0c2-fc16-11e6-8707-c3857a3e4644.png)

#### Stingray Editor

![image](https://cloud.githubusercontent.com/assets/4054655/23341501/cc9a40e4-fc16-11e6-9c71-dd581a142653.png)

## Videos

https://youtu.be/yC7pq5Y_K_Q
