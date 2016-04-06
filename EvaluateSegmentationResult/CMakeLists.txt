PROJECT(EvaluateSegmentationResult)

FIND_PACKAGE(ITK)
IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
    MESSAGE(FATAL_ERROR
    "ITK not found. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

FIND_PATH(ANN_PATH NAMES ANN DOC "Path to ANN include base dir")
FIND_LIBRARY(ANN_LIB NAMES ann PATHS ${ANN_PATH})

INCLUDE_DIRECTORIES(${ANN_PATH})

ADD_EXECUTABLE( EvaluateSegmentationResult EvaluateSegmentationResult.cpp)
TARGET_LINK_LIBRARIES( EvaluateSegmentationResult ITKIO ITKBasicFilters ITKCommon ${ANN_LIB})
