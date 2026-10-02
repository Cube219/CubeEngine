#pragma once

#include "CoreHeader.h"

#include "Vector.h"

namespace cube
{
    class Material;
    class Mesh;

    class SceneObject
    {
    public:
        SceneObject(StringView name, const SharedPtr<Mesh>& mesh);
        ~SceneObject();

        SharedPtr<Mesh> GetMesh() const { return mMesh; }
        const Vector<WeakPtr<Material>>& GetMaterials() const { return mMaterials; }

        Matrix GetTransformMatrix();
        void SetTransformMatrix(const Matrix& transform);

        void SetPosition(Vector3 position);
        void SetRotation(Vector3 rotation);
        void SetScale(Vector3 scale);

        void SetMaterials(ArrayView<WeakPtr<Material>> materials);

    private:
        String mName;

        Vector3 mPosition = Vector3::Zero();
        Vector3 mRotation = Vector3::Zero();
        Vector3 mScale = { 1.0f, 1.0f, 1.0f };
        Matrix mTransformMatrix;
        bool mIsTransformMatrixDirty = true;

        SharedPtr<Mesh> mMesh;
        Vector<WeakPtr<Material>> mMaterials;
    };
} // namespace cube
