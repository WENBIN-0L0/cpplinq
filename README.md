# CPPLINQ TOY

这个玩具是cpp linq的部分实现，没有全部实现完。基本上满足我自己的需要，不够用的话以后再加。参考了几个LINQ for CPP的实现。但是无奈自己公司总在用vs2010，很多版本都多多少少编译有问题，一气之下，自己实现一个来玩玩。

如下测试代码
```c++
template<typename T>
string ToString(const T& t)
{
    ostringstream oss;
    oss << t;
    return oss.str();
}

void test1()
{
    cout << "--test1--" << endl;
    int ia[] = {1,2,3,4,5};
    vector<int> va = from(std::begin(ia),std::end(ia)).select([](int it)->int{return it*2;}).to_vector();
    from(std::begin(va),std::end(va)).foreach([](int it){ cout << it << ",";});
    cout << endl << "-----" << endl;
    num_range(0,10).select([](int it)->int{return it*2;}).where([](int it)->bool{ return it%4==0;}).foreach([](int it)
    {
        cout << it << ",";
    });
    cout << endl;
}

void test2()
{
    cout << "--test2--" << endl;

    string sa = num_range<int>(10,0,-1).select([](int it)->double{return it*2.0;}).where([](double it)->bool{ return it<7;})
        .select([](double it)->string{ return ToString(it);}).aggregate([](string acc,string it)->string{ return acc+","+it;});
    cout << sa <<endl;

    int ia[] = {1,2,3,4,5};
    cout << from(std::begin(ia),std::end(ia)).where([](int it)->bool{ return it > 3;}).first() << endl;
    cout << from(std::begin(ia),std::end(ia)).where([](int it)->bool{ return it > 3;}).firstOrDefault(100) << endl;
    cout << from(std::begin(ia),std::end(ia)).where([](int it)->bool{ return it > 10;}).firstOrDefault(100) << endl;

    auto xx=[](int it)->function<int(int)>{ return [it](int j)->int {return j*it;};};
    string sb = from(std::begin(ia),std::end(ia)).where([](int it)->bool { return it<=4;}).select_many([xx](int it)->vector<int>
    {
        return num_range<int>(0,4).select(xx(it)).to_vector();
    }).aggregate(string(""),[](string acc,int it)->string{ return acc + ToString(it) + ",";});
    cout << sb << endl;
}
```

gcc/vs2010 编译通过,执行结果
```
--test1--
2,4,6,8,10,
-----
0,4,8,12,16,20,
--test2--
6,4,2,0
4
4
100
0,1,2,3,4,0,2,4,6,8,0,3,6,9,12,0,4,8,12,16,
```



