#include "SceneObject.h"

namespace cube
{
    SceneObject::SceneObject(StringView name, const SharedPtr<Mesh>& mesh)
        : mName(name)
        , mMesh(mesh)
    {
    }

    SceneObject::~SceneObject()
    {
        mMesh = nullptr;
    }

    Matrix SceneObject::GetModelMatrix()
    {
        if (mIsModelMatrixDirty)
        {
            mModel = MatrixUtility::GetScale(mScale) * MatrixUtility::GetRotationXYZ(mRotation) + MatrixUtility::GetTranslation_Add(mPosition);

            mIsModelMatrixDirty = false;
        }

        return mModel;
    }

    void SceneObject::SetPosition(Vector3 position)
    {
        mPosition = position;

        mIsModelMatrixDirty = true;
    }

    void SceneObject::SetRotation(Vector3 rotation)
    {
        mRotation = rotation;

        mIsModelMatrixDirty = true;
    }

    void SceneObject::SetScale(Vector3 scale)
    {
        mScale = scale;

        mIsModelMatrixDirty = true;
    }

    void SceneObject::SetMaterials(ArrayView<WeakPtr<Material>> materials)
    {
        mMaterials.clear();
        mMaterials.assign(materials.begin(), materials.end());
    }
} // namespace cube
