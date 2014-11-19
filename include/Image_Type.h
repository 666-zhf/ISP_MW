#ifndef IMAGE_STRUCT_H_
#define IMAGE_STRUCT_H_


#include <vector>
#include <array>
#include <crtdefs.h>
#include "Type.h"
#include "Helper.h"
#include "Specification.h"


const DType MaxBitDepth = sizeof(DType) * 8 * 3 / 4;


enum class PixelType {
    Y = 0,
    U = 1,
    V = 2,
    YUV444 = 3,
    YUV422 = 4,
    YUV420 = 5,
    YUV411 = 6,
    R = 7,
    G = 8,
    B = 9,
    RGB = 10
};

enum class ChromaPlacement {
    MPEG1 = 0,
    MPEG2 = 1,
    DV    = 2
};

enum class QuantRange {
    TV = 0,
    PC = 1
};


class Plane;
class Plane_FL;
class Frame;


class Plane {
    typedef DType _Ty;

public:
    typedef Plane _Myt;
    typedef _Ty value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Ty *pointer;
    typedef const _Ty *const_pointer;
    typedef _Ty& reference;
    typedef const _Ty& const_reference;

    typedef pointer iterator;
    typedef const_pointer const_iterator;

public:
    const value_type value_type_MIN = value_type(0);
    const value_type value_type_MAX = value_type_MIN - 1;
    
private:
    PCType Width_ = 0;
    PCType Height_ = 0;
    PCType PixelCount_ = 0;
    value_type BitDepth_;
    value_type Floor_;
    value_type Neutral_;
    value_type Ceil_;
    value_type ValueRange_;
    TransferChar TransferChar_;
    pointer Data_ = nullptr;

protected:
    void DefaultPara(bool Chroma, value_type BitDepth = 16, QuantRange _QuantRange = QuantRange::PC);
    void CopyParaFrom(const _Myt& src);
    void InitValue(value_type Value, bool Init = true);

public:
    explicit _Myt(value_type Value = 0, PCType Width = 1920, PCType Height = 1080, value_type BitDepth = 16, bool Init = true); // Default constructor and Convertor/Constructor from value_type
    _Myt(value_type Value, PCType Width, PCType Height, value_type BitDepth, value_type Floor, value_type Neutral, value_type Ceil, TransferChar _TransferChar, bool Init = true);

    _Myt(const _Myt& src); // Copy constructor
    _Myt(const _Myt& src, bool Init, value_type Value = 0);
    _Myt(_Myt&& src); // Move constructor
    explicit _Myt(const Plane_FL& src, value_type BitDepth = 16); // Convertor/Constructor from Plane_FL
    _Myt(const Plane_FL& src, value_type BitDepth, value_type Floor, value_type Neutral, value_type Ceil);
    _Myt(const Plane_FL& src, bool Init, value_type Value = 0, value_type BitDepth = 16);
    _Myt(const Plane_FL& src, bool Init, value_type Value, value_type BitDepth, value_type Floor, value_type Neutral, value_type Ceil);

    ~Plane(); // Destructor

    _Myt& operator=(const _Myt& src); // Copy assignment operator
    _Myt& operator=(_Myt&& src); // Move assignment operator
    bool operator==(const _Myt& b) const;
    bool operator!=(const _Myt& b) const { return !(*this == b); }
    reference operator[](PCType i) { return Data_[i]; }
    const_reference operator[](PCType i) const { return Data_[i]; }

    iterator begin() { return Data_; }
    const_iterator begin() const { return Data_; }
    iterator end() { return Data_ + PixelCount_; }
    const_iterator end() const { return Data_ + PixelCount_; }
    size_type size() const { return PixelCount_; }
    pointer data() { return Data_; }
    const_pointer data() const { return Data_; }
    value_type value(PCType i) { return Data_[i]; }
    const value_type value(PCType i) const { return Data_[i]; }

    PCType Height() const { return Height_; }
    PCType Width() const { return Width_; }
    PCType Stride() const { return Width_; }
    PCType PixelCount() const { return PixelCount_; }
    value_type BitDepth() const { return BitDepth_; }
    value_type Floor() const { return Floor_; }
    value_type Neutral() const { return Neutral_; }
    value_type Ceil() const { return Ceil_; }
    value_type ValueRange() const { return ValueRange_; }
    TransferChar GetTransferChar() const { return TransferChar_; }

    pointer Data() { return Data_; }
    const_pointer Data() const { return Data_; }

    bool isChroma() const { return Floor_ < Neutral_; }
    bool isPCChroma() const { return Floor_ < Neutral_ && (Floor_ + Ceil_) % 2 == 1; }
    value_type Min() const;
    value_type Max() const;
    void MinMax(reference min, reference max) const;
    FLType Mean() const;
    FLType Variance(FLType Mean) const;
    FLType Variance() const { return Variance(Mean()); }

    _Myt& Width(PCType Width) { return ReSize(Width, Height_); }
    _Myt& Height(PCType Height) { return ReSize(Width_, Height); }
    _Myt& ReSize(PCType Width, PCType Height);
    _Myt& ReQuantize(value_type BitDepth = 16, QuantRange _QuantRange = QuantRange::PC, bool scale = true, bool clip = false);
    _Myt& ReQuantize(value_type BitDepth, value_type Floor, value_type Neutral, value_type Ceil, bool scale = true, bool clip = false);
    _Myt& SetTransferChar(TransferChar _TransferChar) { TransferChar_ = _TransferChar; return *this; }

    _Myt& From(const Plane& src);
    _Myt& From(const Plane_FL& src);
    _Myt& ConvertFrom(const Plane& src, TransferChar dstTransferChar);
    _Myt& ConvertFrom(const Plane& src) { return ConvertFrom(src, TransferChar_); }
    _Myt& YFrom(const Frame& src);
    _Myt& YFrom(const Frame& src, ColorMatrix dstColorMatrix);

    FLType GetFL(value_type input) const { return static_cast<FLType>(input - Neutral_) / ValueRange_; }
    FLType GetFL_PCChroma(value_type input) const { return Clip(static_cast<FLType>(input - Neutral_) / ValueRange_, -0.5, 0.5); }
    value_type GetD(FLType input) const { return static_cast<value_type>(input*ValueRange_ + Neutral_ + FLType(0.5)); }
    value_type GetD_PCChroma(FLType input) const { return static_cast<value_type>(input*ValueRange_ + Neutral_ + FLType(0.499999)); }

    _Myt& Binarize(const _Myt& src, value_type lower_thrD, value_type upper_thrD);
    _Myt& Binarize(value_type lower_thrD, value_type upper_thrD) { return Binarize(*this, lower_thrD, upper_thrD); }
    _Myt& Binarize_ratio(const _Myt& src, double lower_thr = 0., double upper_thr = 1.);
    _Myt& Binarize_ratio(double lower_thr = 0., double upper_thr = 1.) { return Binarize_ratio(*this, lower_thr, upper_thr); }
    _Myt& SimplestColorBalance(Plane_FL& flt, const Plane& src, double lower_thr = 0., double upper_thr = 0., int HistBins = 4096);

    template < typename T > value_type Quantize(T input) const;

    template < typename _Fn1 > void for_each(_Fn1 _Func) const;
    template < typename _Fn1 > void for_each(_Fn1 _Func);
    template < typename _Fn1 > void transform(_Fn1 _Func);
    template < typename _St1, typename _Fn1 > void transform(const _St1& src, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _Fn1 > void transform(const _St1& src1, const _St2& src2, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _St3, typename _Fn1 > void transform(const _St1& src1, const _St2& src2, const _St3& src3, _Fn1 _Func);
    template < PCType VRad, PCType HRad, typename _St1, typename _Fn1 > void convolute(const _St1& src, _Fn1 _Func);

    template < typename _Fn1 > void for_each_PPL(_Fn1 _Func) const;
    template < typename _Fn1 > void for_each_PPL(_Fn1 _Func);
    template < typename _Fn1 > void transform_PPL(_Fn1 _Func);
    template < typename _St1, typename _Fn1 > void transform_PPL(const _St1& src, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _Fn1 > void transform_PPL(const _St1& src1, const _St2& src2, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _St3, typename _Fn1 > void transform_PPL(const _St1& src1, const _St2& src2, const _St3& src3, _Fn1 _Func);
    template < PCType VRad, PCType HRad, typename _St1, typename _Fn1 > void convolute_PPL(const _St1& src, _Fn1 _Func);

    template < typename _Fn1 > void for_each_AMP(_Fn1 _Func) const;
    template < typename _Fn1 > void for_each_AMP(_Fn1 _Func);
    template < typename _Fn1 > void transform_AMP(_Fn1 _Func);
    template < typename _St1, typename _Fn1 > void transform_AMP(const _St1& src, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _Fn1 > void transform_AMP(const _St1& src1, const _St2& src2, _Fn1 _Func);
    template < typename _St1, typename _St2, typename _St3, typename _Fn1 > void transform_AMP(const _St1& src1, const _St2& src2, const _St3& src3, _Fn1 _Func);
    template < PCType VRad, PCType HRad, typename _St1, typename _Fn1 > void convolute_AMP(const _St1& src, _Fn1 _Func);
};


class Plane_FL {
    typedef FLType _Ty;

public:
    typedef Plane_FL _Myt;
    typedef _Ty value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Ty *pointer;
    typedef const _Ty *const_pointer;
    typedef _Ty& reference;
    typedef const _Ty& const_reference;

    typedef pointer iterator;
    typedef const_pointer const_iterator;

public:
    const value_type value_type_MIN = sizeof(value_type) < 8 ? FLT_MIN : DBL_MIN;
    const value_type value_type_MAX = sizeof(value_type) < 8 ? FLT_MAX : DBL_MAX;

private:
    PCType Width_ = 0;
    PCType Height_ = 0;
    PCType PixelCount_ = 0;
    value_type Floor_ = 0;
    value_type Neutral_ = 0;
    value_type Ceil_ = 1;
    TransferChar TransferChar_;
    pointer Data_ = nullptr;

protected:
    void DefaultPara(bool Chroma, value_type range = 1);
    void CopyParaFrom(const _Myt& src);
    void InitValue(value_type Value, bool Init = true);

public:
    explicit _Myt(value_type Value = 0, PCType Width = 1920, PCType Height = 1080, bool RGB = true, bool Chroma = false, bool Init = true); // Default constructor and Convertor/Constructor from value_type
    _Myt(value_type Value, PCType Width, PCType Height, value_type Floor, value_type Neutral, value_type Ceil, TransferChar _TransferChar, bool Init = true);

    _Myt(const _Myt& src); // Copy constructor
    _Myt(const _Myt& src, bool Init, value_type Value = 0);
    _Myt(_Myt&& src); // Move constructor
    explicit _Myt(const Plane& src, value_type range = 1.); // Convertor/Constructor from Plane
    _Myt(const Plane& src, bool Init, value_type Value = 0, value_type range = 1.);
    _Myt(const _Myt& src, TransferChar dstTransferChar);
    _Myt(const Plane& src, TransferChar dstTransferChar);

    ~Plane_FL(); // Destructor

    _Myt& operator=(const _Myt& src); // Copy assignment operator
    _Myt& operator=(_Myt&& src); // Move assignment operator
    bool operator==(const _Myt& b) const;
    bool operator!=(const _Myt& b) const { return !(*this == b); }
    reference operator[](PCType i) { return Data_[i]; }
    const_reference operator[](PCType i) const { return Data_[i]; }

    iterator begin() { return Data_; }
    const_iterator begin() const { return Data_; }
    iterator end() { return Data_ + PixelCount_; }
    const_iterator end() const { return Data_ + PixelCount_; }
    size_type size() const { return PixelCount_; }
    pointer data() { return Data_; }
    const_pointer data() const { return Data_; }
    value_type value(PCType i) { return Data_[i]; }
    const value_type value(PCType i) const { return Data_[i]; }

    PCType Height() const { return Height_; }
    PCType Width() const { return Width_; }
    PCType Stride() const { return Width_; }
    PCType PixelCount() const { return PixelCount_; }
    value_type Floor() const { return Floor_; }
    value_type Neutral() const { return Neutral_; }
    value_type Ceil() const { return Ceil_; }
    value_type ValueRange() const { return Ceil_ - Floor_; }
    TransferChar GetTransferChar() const { return TransferChar_; }

    pointer Data() { return Data_; }
    const_pointer Data() const { return Data_; }

    bool isChroma() const { return Floor_ < Neutral_; }
    value_type Min() const;
    value_type Max() const;
    void MinMax(reference min, reference max) const;
    value_type Mean() const;
    value_type Variance(value_type Mean) const;
    value_type Variance() const { return Variance(Mean()); }

    _Myt& Width(PCType Width) { return ReSize(Width, Height_); }
    _Myt& Height(PCType Height) { return ReSize(Width_, Height); }
    _Myt& ReSize(PCType Width, PCType Height);
    _Myt& ReQuantize(value_type Floor, value_type Neutral, value_type Ceil, bool scale = true, bool clip = false);
    _Myt& SetTransferChar(TransferChar _TransferChar) { TransferChar_ = _TransferChar; return *this; }

    _Myt& From(const Plane& src);
    _Myt& ConvertFrom(const Plane_FL& src, TransferChar dstTransferChar);
    _Myt& ConvertFrom(const Plane_FL& src) { return ConvertFrom(src, TransferChar_); }
    _Myt& ConvertFrom(const Plane& src, TransferChar dstTransferChar);
    _Myt& ConvertFrom(const Plane& src) { return ConvertFrom(src, TransferChar_); }
    _Myt& YFrom(const Frame& src);
    _Myt& YFrom(const Frame& src, ColorMatrix dstColorMatrix);

    _Myt& Binarize(const _Myt& src, value_type lower_thrD, value_type upper_thrD);
    _Myt& Binarize(value_type lower_thrD, value_type upper_thrD) { return Binarize(*this, lower_thrD, upper_thrD); }
    _Myt& Binarize_ratio(const _Myt& src, double lower_thr = 0., double upper_thr = 1.);
    _Myt& Binarize_ratio(double lower_thr = 0., double upper_thr = 1.) { return Binarize_ratio(*this, lower_thr, upper_thr); }
    _Myt& SimplestColorBalance(const _Myt& flt, const _Myt& src, double lower_thr = 0., double upper_thr = 0., int HistBins = 4096);

    template < typename T > value_type Quantize(T input) const;
};


class Frame {
    typedef DType _Ty;

public:
    typedef Frame _Myt;
    typedef Plane _Mysub;
    typedef _Ty value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Ty *pointer;
    typedef const _Ty *const_pointer;
    typedef _Ty& reference;
    typedef const _Ty& const_reference;

    typedef pointer iterator;
    typedef const_pointer const_iterator;

    typedef sint32 PlaneCountType;
    static const PlaneCountType MaxPlaneCount = 7;

private:
    FCType FrameNum_;
    PixelType PixelType_;
    QuantRange QuantRange_;
    ChromaPlacement ChromaPlacement_;
    ColorPrim ColorPrim_;
    TransferChar TransferChar_;
    ColorMatrix ColorMatrix_;

    PlaneCountType PlaneCount_ = 0;
    std::vector<_Mysub *> P_;

    _Mysub *R_ = nullptr;
    _Mysub *G_ = nullptr;
    _Mysub *B_ = nullptr;
    _Mysub *Y_ = nullptr;
    _Mysub *U_ = nullptr;
    _Mysub *V_ = nullptr;
    _Mysub *A_ = nullptr;

protected:
    bool isYUV(PixelType _PixelType) const { return _PixelType >= PixelType::Y && _PixelType < PixelType::R; }
    bool isRGB(PixelType _PixelType) const { return _PixelType >= PixelType::R && _PixelType <= PixelType::RGB; }

    void InitPlanes(PCType Width = 1920, PCType Height = 1080, value_type BitDepth = 16, bool Init = true);
    void CopyPlanes(const _Myt& src, bool Copy = true, bool Init = false);
    void MovePlanes(_Myt& src);
    void FreePlanes();

public:
    explicit _Myt(FCType FrameNum = 0, PixelType _PixelType = PixelType::RGB, PCType Width = 1920, PCType Height = 1080,
        value_type BitDepth = 16, bool Init = true); // Default constructor and Convertor/Constructor from FCType
    _Myt(FCType FrameNum, PixelType _PixelType, PCType Width, PCType Height, value_type BitDepth,
        QuantRange _QuantRange, ChromaPlacement _ChromaPlacement = ChromaPlacement::MPEG2, bool Init = true);
    _Myt(FCType FrameNum, PixelType _PixelType, PCType Width, PCType Height, value_type BitDepth, QuantRange _QuantRange,
        ChromaPlacement _ChromaPlacement, ColorPrim _ColorPrim, TransferChar _TransferChar, ColorMatrix _ColorMatrix, bool Init = true);

    _Myt(const _Myt& src, bool Copy = true, bool Init = false); // Copy constructor
    _Myt(_Myt&& src); // Move constructor

    ~Frame(); // Destructor

    _Myt& operator=(const _Myt& src); // Copy assignment operator
    _Myt& operator=(_Myt&& src); // Move assignment operator
    bool operator==(const _Myt& b) const;
    bool operator!=(const _Myt& b) const { return !(*this == b); }

    FCType FrameNum() const { return FrameNum_; }
    PixelType GetPixelType() const { return PixelType_; }
    ChromaPlacement GetChromaPlacement() const { return ChromaPlacement_; }
    QuantRange GetQuantRange() const { return QuantRange_; }
    ColorPrim GetColorPrim() const { return ColorPrim_; }
    TransferChar GetTransferChar() const { return TransferChar_; }
    ColorMatrix GetColorMatrix() const { return ColorMatrix_; }
    PlaneCountType PlaneCount() const { return PlaneCount_; }

    _Mysub& P(PlaneCountType PlaneNum) { return *P_[PlaneNum]; }
    _Mysub& R() { return *R_; }
    _Mysub& G() { return *G_; }
    _Mysub& B() { return *B_; }
    _Mysub& Y() { return *Y_; }
    _Mysub& U() { return *U_; }
    _Mysub& V() { return *V_; }
    _Mysub& A() { return *A_; }
    const _Mysub& P(PlaneCountType PlaneNum) const { return *P_[PlaneNum]; }
    const _Mysub& R() const { return *R_; }
    const _Mysub& G() const { return *G_; }
    const _Mysub& B() const { return *B_; }
    const _Mysub& Y() const { return *Y_; }
    const _Mysub& U() const { return *U_; }
    const _Mysub& V() const { return *V_; }
    const _Mysub& A() const { return *A_; }

    bool isYUV() const { return PixelType_ >= PixelType::Y && PixelType_ < PixelType::R; }
    bool isRGB() const { return PixelType_ >= PixelType::R && PixelType_ <= PixelType::RGB; }

    PCType Height() const { return P_[0]->Height(); }
    PCType Width() const { return P_[0]->Width(); }
    PCType Stride() const { return P_[0]->Stride(); }
    PCType PixelCount() const { return P_[0]->PixelCount(); }
    value_type BitDepth() const { return P_[0]->BitDepth(); }

    _Myt& ConvertFrom(const Frame& src, TransferChar dstTransferChar);
};


#include "Image_Type.hpp"


#endif