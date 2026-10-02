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

    Matrix SceneObject::GetTransformMatrix()
    {
        if (mIsTransformMatrixDirty)
        {
            mTransformMatrix = MatrixUtility::GetScale(mScale) * MatrixUtility::GetRotationXYZ(mRotation) + MatrixUtility::GetTranslation_Add(mPosition);

            mIsTransformMatrixDirty = false;
        }

        return mTransformMatrix;
    }

    void SceneObject::SetTransformMatrix(const Matrix& transform)
    {
        mTransformMatrix = transform;
        mIsTransformMatrixDirty = false;

        MatrixUtility::DecomposeTransformation(mTransformMatrix, mPosition, mRotation, mScale);
    }

    void SceneObject::SetPosition(Vector3 position)
    {
        mPosition = position;

        mIsTransformMatrixDirty = true;
    }

    void SceneObject::SetRotation(Vector3 rotation)
    {
        mRotation = rotation;

        mIsTransformMatrixDirty = true;
    }

    void SceneObject::SetScale(Vector3 scale)
    {
        mScale = scale;

        mIsTransformMatrixDirty = true;
    }

    void SceneObject::SetMaterials(ArrayView<WeakPtr<Material>> materials)
    {
        mMaterials.clear();
        mMaterials.assign(materials.begin(), materials.end());
    }
} // namespace cube
