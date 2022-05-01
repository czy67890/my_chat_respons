#pragma once
#include<functional>
#include<memory>

namespace czy{
template<typename CLASS,typename... ARGS>
class WeakCallback{
public:
    WeakCallback(const std::weak_ptr<CALSS>&object,const std::function<void(ClASS *,ARGS...)> &func)
     :object_(object),function_(func)
    {}
    //&&创造万能引用
    void operator()(ARGS&& .. args) const{
        std::shared_ptr<CLASS> ptr(object_.lock());
        if(ptr){
            function_(ptr.get(),std::forward<ARGS>(args)...);
        }
    }
private:
    //weak_ptr与shared_ptr搭配使用
    std::weak_ptr<CLASS> object_;
    std::function<void (CLASS*,ARGS...)> function_;
};

template<typename CLASS,typename... ARGS>
WeakCallback<CLASS,ARGS...> makeWeakCallback(const std::shared_ptr<CLASS> &object,void(CLASS::*function)(ARGS...)){
    return WeakCallcack(object,function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
  return WeakCallback<CLASS, ARGS...>(object, function);
}
}