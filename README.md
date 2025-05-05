Project VR
-
Group Members (GROUP 5)
- 
  - Sorawis Pruckwattananon
  - Chawasit Pattharaphongplin
  - Sujira Puenphan
    
Overview: 
-
This project aimed to develop a PC and Virtual Reality (VR) application using C++, Qt, and VTK, allowing users to interactively view, organize, and edit CAD models of the Formula Student car. The final product includes both desktop and VR functionality, creating an immersive, intuitive 3D environment to explore engineering designs.

Objectives:
-
  - Build a base-station GUI in Qt to load, display, and manipulate CAD assemblies.
  - Integrate VTK for 3D model rendering.
  - Implement VR support using OpenVR and extend the rendering to HTC Vive Pro 2 headsets.
  - Provide interactive features such as model tree management, visibility toggling, and model filtering.
    
Features:
-
  1. Qt-Based GUI: 
    - TreeView displaying a hierarchical structure of model components.
    - Menus and toolbars to manage component visibility, color, and naming.
    - Drag-and-drop loading of multiple CAD files.
    - Context menus for component-specific actions.
  2. 3D Rendering with VTK:
    - Real-time visualization of CAD models.
    - Custom shaders and lighting effects.
    - Support for applying Clip Filters and Shrink Filters.
  3. VR Integration:
    - Separate rendering thread for VR using VRRenderThread.
    - Independent actor creation for VR scenes, mirroring the desktop setup.
    - Accurate 6DoF tracking and room-scale rendering.
  4. Installer and Documentation:
    - Installer generated with NSIS, including all required DLLs.
    - Automatic documentation generated with Doxygen and deployed to GitHub Pages.
     
Technologies Used:
-
  - Languages: C++
  - Libraries: Qt 6.8.2, VTK 9.4.2, OpenVR
  - Tools: CMake, Visual Studio 2022, GitHub, NSIS
    
Learning Outcomes:
-
  Our knowledge in cross-platform GUI development, 3D rendering, VR application design, Git version control, and building reliable installers was enhanced by this project. Additionally, it strengthened ideas in real-time graphics, object-oriented programming, and user interface design.
  
Conclusion:
-
  The finished VR-enabled program exhibits efficient rendering and interaction with intricate CAD models, making it appropriate for both educational display and engineering inspection. Future expansions, such as physics simulation or remote cooperation, are made possible by the modular architecture and clear documentation.
