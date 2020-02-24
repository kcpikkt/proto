// old, silly, way of handling errors
using ErrCode = s32;
using ErrMessage = const char *;

template<typename Category>
struct Err {
    //    static_assert(meta::is_base_of_v<ErrCategoryCRTP<Category>, Category>);

    s32 code;
    operator ErrCode() { return code; }

    Err(ErrCode code) : code(code) {}

    ErrMessage message() {
        return Category::message(code);
    }
};

template<typename T>
struct ErrCategoryCRTP {
    static ErrMessage message(ErrCode code) {
        return T::message(code);
    }
};

