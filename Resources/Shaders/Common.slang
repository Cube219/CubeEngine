module Common;

#if CUBE_SLANG_METAL
// Currently Slang does not support DescriptorHandle in Metal and has some crash while compile.
// So use type itself and set gpuResouceID directly. (Required argument buffer Tier 2)
public typealias Bindless<T> where T:IOpaqueDescriptor = T;
#else
public typealias Bindless<T> where T:IOpaqueDescriptor = DescriptorHandle<T>;
#endif
