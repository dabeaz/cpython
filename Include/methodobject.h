
/* Method object interface */

#ifndef Py_METHODOBJECT_H
#define Py_METHODOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif

/* This is about the type 'builtin_function_or_method',
   not Python methods in user-defined classes.  See classobject.h
   for the latter. */

PyAPI_DATA(PyTypeObject) PyCFunction_Type;

  
PyAPI_FUNC(int) PyCFunction_CheckExact(PyObject *op);
PyAPI_FUNC(int) PyCFunction_Check(PyObject *op);

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);
typedef PyObject *(*_PyCFunctionFast) (PyObject *, PyObject *const *, Py_ssize_t);
typedef PyObject *(*PyCFunctionWithKeywords)(PyObject *, PyObject *,
                                             PyObject *);
typedef PyObject *(*_PyCFunctionFastWithKeywords) (PyObject *,
                                                   PyObject *const *, Py_ssize_t,
                                                   PyObject *);
typedef PyObject *(*PyCMethod)(PyObject *, PyTypeObject *, PyObject *const *,
                               size_t, PyObject *);

PyAPI_FUNC(PyCFunction) PyCFunction_GetFunction(PyObject *);
PyAPI_FUNC(PyObject *) PyCFunction_GetSelf(PyObject *);
PyAPI_FUNC(int) PyCFunction_GetFlags(PyObject *);
PyAPI_FUNC(const char *) PyCFunction_GetName(PyObject *func);

struct PyMethodDef {
    const char  *ml_name;   /* The name of the built-in function/method */
    PyCFunction ml_meth;    /* The C function that implements it */
    int         ml_flags;   /* Combination of METH_xxx flags, which mostly
                               describe the args expected by the C func */
    const char  *ml_doc;    /* The __doc__ attribute, or NULL */
};
typedef struct PyMethodDef PyMethodDef;

#define PyCFunction_New(ML, SELF) PyCFunction_NewEx((ML), (SELF), NULL)
PyAPI_FUNC(PyObject *) PyCFunction_NewEx(PyMethodDef *, PyObject *,
                                         PyObject *);

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03090000
#define PyCFunction_NewEx(ML, SELF, MOD) PyCMethod_New((ML), (SELF), (MOD), NULL)
PyAPI_FUNC(PyObject *) PyCMethod_New(PyMethodDef *, PyObject *,
                                     PyObject *, PyTypeObject *);
#endif


/* Flag passed to newmethodobject */
/* #define METH_OLDARGS  0x0000   -- unsupported now */
#define METH_VARARGS  0x0001
#define METH_KEYWORDS 0x0002
/* METH_NOARGS and METH_O must not be combined with the flags above. */
#define METH_NOARGS   0x0004
#define METH_O        0x0008

/* METH_CLASS and METH_STATIC are a little different; these control
   the construction of methods for a class.  These cannot be used for
   functions in modules. */
#define METH_CLASS    0x0010
#define METH_STATIC   0x0020

/* METH_COEXIST allows a method to be entered even though a slot has
   already filled the entry.  When defined, the flag allows a separate
   method, "__contains__" for example, to coexist with a defined
   slot like sq_contains. */

#define METH_COEXIST   0x0040

#ifndef Py_LIMITED_API
#define METH_FASTCALL  0x0080
#endif

/* METH_METHOD means the function stores an
 * additional reference to the class that defines it;
 * both self and class are passed to it.
 * It uses PyCMethodObject instead of PyCFunctionObject.
 * May not be combined with METH_NOARGS, METH_O, METH_CLASS or METH_STATIC.
 */

#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03090000
#define METH_METHOD 0x0200
#endif
  
PyAPI_DATA(PyTypeObject) PyCMethod_Type;

#define PyCMethod_CheckExact(op) Py_IS_TYPE(op, &PyCMethod_Type)
#define PyCMethod_Check(op) PyObject_TypeCheck(op, &PyCMethod_Type)
  
#ifdef __cplusplus
}
#endif
#endif /* !Py_METHODOBJECT_H */
