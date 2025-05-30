/*
Open Asset Import Library (assimp)
----------------------------------------------------------------------

Copyright (c) 2006-2025, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the
following conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----------------------------------------------------------------------
*/

/** @file ColladaExporter.h
 * Declares the exporter class to write a scene to a Collada file
 */
#ifndef AI_COLLADAEXPORTER_H_INC
#define AI_COLLADAEXPORTER_H_INC

#include <assimp/ai_assert.h>
#include <assimp/material.h>

#include <array>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

struct aiScene;
struct aiNode;
struct aiLight;
struct aiBone;

namespace Assimp {

class IOSystem;

/// Helper class to export a given scene to a Collada file. Just for my personal
/// comfort when implementing it.
class ColladaExporter {
public:
    /// Constructor for a specific scene to export
    ColladaExporter(const aiScene *pScene, IOSystem *pIOSystem, const std::string &path, const std::string &file);

    /// Destructor
    virtual ~ColladaExporter() = default;

protected:
    /// Starts writing the contents
    void WriteFile();

    /// Writes the asset header
    void WriteHeader();

    /// Writes the embedded textures
    void WriteTextures();

    /// Writes the material setup
    void WriteMaterials();

    /// Writes the cameras library
    void WriteCamerasLibrary();

    // Write a camera entry
    void WriteCamera(size_t pIndex);

    /// Writes the cameras library
    void WriteLightsLibrary();

    // Write a camera entry
    void WriteLight(size_t pIndex);
    void WritePointLight(const aiLight *const light);
    void WriteDirectionalLight(const aiLight *const light);
    void WriteSpotLight(const aiLight *const light);
    void WriteAmbientLight(const aiLight *const light);

    /// Writes the controller library
    void WriteControllerLibrary();

    /// Writes a skin controller of the given mesh
    void WriteController(size_t pIndex);

    /// Writes the geometry library
    void WriteGeometryLibrary();

    /// Writes the given mesh
    void WriteGeometry(size_t pIndex);

    //enum FloatDataType { FloatType_Vector, FloatType_TexCoord2, FloatType_TexCoord3, FloatType_Color, FloatType_Mat4x4, FloatType_Weight };
    // customized to add animation related type
    enum FloatDataType { FloatType_Vector,
        FloatType_TexCoord2,
        FloatType_TexCoord3,
        FloatType_Color,
        FloatType_Mat4x4,
        FloatType_Weight,
        FloatType_Time };

    /// Writes a float array of the given type
    void WriteFloatArray(const std::string &pIdString, FloatDataType pType, const ai_real *pData, size_t pElementCount);

    /// Writes the scene library
    void WriteSceneLibrary();

    // customized, Writes the animation library
    void WriteAnimationsLibrary();
    void WriteAnimationLibrary(size_t pIndex);
    std::string mFoundSkeletonRootNodeID = "skeleton_root"; // will be replaced by found node id in the WriteNode call.

    /// Recursively writes the given node
    void WriteNode(const aiNode *pNode);

    /// Enters a new xml element, which increases the indentation
    void PushTag() { startstr.append("  "); }
    /// Leaves an element, decreasing the indentation
    void PopTag() {
        ai_assert(startstr.length() > 1);
        startstr.erase(startstr.length() - 2);
    }

    void CreateNodeIds(const aiNode *node);

    /// Get or Create a unique Node ID string for the given Node
    std::string GetNodeUniqueId(const aiNode *node);
    std::string GetNodeName(const aiNode *node);

    std::string GetBoneUniqueId(const aiBone *bone);

    enum class AiObjectType {
        Mesh,
        Material,
        Animation,
        Light,
        Camera,
        Count,
    };
    /// Get or Create a unique ID string for the given scene object index
    std::string GetObjectUniqueId(AiObjectType type, size_t pIndex);
    /// Get or Create a name string for the given scene object index
    std::string GetObjectName(AiObjectType type, size_t pIndex);

    typedef std::map<size_t, std::string> IndexIdMap;
    typedef std::pair<std::string, std::string> NameIdPair;
    NameIdPair AddObjectIndexToMaps(AiObjectType type, size_t pIndex);

    // Helpers
    inline IndexIdMap &GetObjectIdMap(AiObjectType type) { return mObjectIdMap[static_cast<size_t>(type)]; }
    inline IndexIdMap &GetObjectNameMap(AiObjectType type) { return mObjectNameMap[static_cast<size_t>(type)]; }

private:
    std::unordered_set<std::string> mUniqueIds; // Cache of used unique ids
    std::map<const void *, std::string> mNodeIdMap; // Cache of encoded node and bone ids
    std::array<IndexIdMap, static_cast<size_t>(AiObjectType::Count)> mObjectIdMap; // Cache of encoded unique IDs
    std::array<IndexIdMap, static_cast<size_t>(AiObjectType::Count)> mObjectNameMap; // Cache of encoded names

public:
    /// Stringstream to write all output into
    std::stringstream mOutput;

    /// The IOSystem for output
    IOSystem *mIOSystem;

    /// Path of the directory where the scene will be exported
    const std::string mPath;

    /// Name of the file (without extension) where the scene will be exported
    const std::string mFile;

    /// The scene to be written
    const aiScene *const mScene;
    std::string mSceneId;
    bool mAdd_root_node = false;

    /// current line start string, contains the current indentation for simple stream insertion
    std::string startstr;
    /// current line end string for simple stream insertion
    const std::string endstr;

    // pair of color and texture - texture precedences color
    struct Surface {
        bool exist;
        aiColor4D color;
        std::string texture;
        size_t channel;
        Surface() {
            exist = false;
            channel = 0;
        }
    };

    struct Property {
        bool exist;
        ai_real value;
        Property() :
                exist(false),
                value(0.0) {}
    };

    // summarize a material in an convenient way.
    struct Material {
        std::string id;
        std::string name;
        std::string shading_model;
        Surface ambient, diffuse, specular, emissive, reflective, transparent, normal;
        Property shininess, transparency, index_refraction;

        Material() = default;
    };

    std::map<unsigned int, std::string> textures;

public:
    /// Dammit C++ - y u no compile two-pass? No I have to add all methods below the struct definitions
    /// Reads a single surface entry from the given material keys
    bool ReadMaterialSurface(Surface &poSurface, const aiMaterial &pSrcMat, aiTextureType pTexture, const char *pKey, size_t pType, size_t pIndex);
    /// Writes an image entry for the given surface
    void WriteImageEntry(const Surface &pSurface, const std::string &imageId);
    /// Writes the two parameters necessary for referencing a texture in an effect entry
    void WriteTextureParamEntry(const Surface &pSurface, const std::string &pTypeName, const std::string &materialId);
    /// Writes a color-or-texture entry into an effect definition
    void WriteTextureColorEntry(const Surface &pSurface, const std::string &pTypeName, const std::string &imageId);
    /// Writes a scalar property
    void WriteFloatEntry(const Property &pProperty, const std::string &pTypeName);
};

} // namespace Assimp

#endif // !! AI_COLLADAEXPORTER_H_INC
