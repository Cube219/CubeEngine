#include "SceneObject.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

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

    void SceneObject::SetTransformMatrix(const Matrix& transform)
    {
        mTransformMatrix = transform;
        mIsTransformMatrixDirty = false;
        mLastDirtyTransformRenderingFrame = Engine::GetRenderer()->GetCurrentRenderingFrame();

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

    void SceneObject::UpdateDirtyTransform(Uint64 renderingFrame)
    {
        if (mIsTransformMatrixDirty)
        {
            mTransformMatrix = MatrixUtility::GetScale(mScale) * MatrixUtility::GetRotationXYZ(mRotation) + MatrixUtility::GetTranslation_Add(mPosition);

            mIsTransformMatrixDirty = false;
            mLastDirtyTransformRenderingFrame = renderingFrame;
        }
    }

    bool SceneObject::IsRecentlyChangedTransform(Uint64 renderingFrame) const
    {
        return renderingFrame <= mLastDirtyTransformRenderingFrame;
    }

    void SceneObject::SetMaterials(ArrayView<WeakPtr<Material>> materials)
    {
        mMaterials.clear();
        mMaterials.assign(materials.begin(), materials.end());
    }
} // namespace cube
