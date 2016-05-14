/************************************************************************/
/* �������ı����ǣ���ΪSTL�ȵ�����ָ��ʹ������Ҫ��ϸߣ�����ֻҪ��    */
/* ʵ���˳��������Զ�����Ϊ�˸��õĿɿ��ԣ�ʵ������򵥵���           */
/************************************************************************/

#ifndef CLASSIC_PTR_HELPER_H
#define CLASSIC_PTR_HELPER_H

//T must be a pointer type
template<typename T>
class ClassicPtrHelper
{
public:
  ClassicPtrHelper<T>(T *p)
    : ptr_(p)
  {
  }

  ~ClassicPtrHelper<T>()
  {
    if (ptr_ && *ptr_) delete (*ptr_);
  }

private:
  T *ptr_;
};

#endif
