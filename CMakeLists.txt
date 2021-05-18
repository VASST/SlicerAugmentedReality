cmake_minimum_required(VERSION 3.5)

project(SlicerAugmentedReality)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/AugmentedReality")
set(EXTENSION_CATEGORY "Virtual Reality")
set(EXTENSION_CONTRIBUTORS "Adam Rankin (Robarts Research Institute)")
set(EXTENSION_DESCRIPTION "This extensions provides the infrastructure to develop augmented reality applications")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SlicerAugmentedReality.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/AugmentedReality/Screenshots/1.png")
set(EXTENSION_DEPENDS "SlicerVirtualReality SlicerPinholeCameras") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(SlicerVirtualReality REQUIRED)
mark_as_advanced(SlicerVirtualReality_DIR)

find_package(SlicerPinholeCameras REQUIRED)
mark_as_advanced(SlicerPinholeCameras_DIR)

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(VideoPassthrough)
add_subdirectory(TrackedScreenAR)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
