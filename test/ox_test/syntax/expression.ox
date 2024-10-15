ref "../test"
ref "std/log"

log: Log("expression")

test(!true == false)
test(!false == true)
test(!0 == true)
test(!1 == false)
test(!"" == true)
test(!"null" == false)

test(+0 == 0)
test(+1 == 1)
test(+"1" == 1)
test(+"3.1415926" == 3.1415926)

test(-0 == 0)
test(-"1" == -1)
test(-"3.1415926" == -3.1415926)

test(~0 == 0xffffffff)
test(~-1 == 0)
test(~0xffff0000 == 0xffff)

test(1/0 == Number.INFINITY)
test(-1/0 -- -Number.INFINITY)

test(0**1 == 0)
test(1**1 == 1)
test(2**1 == 2)
test(0**0 == 1)
test(1**0 == 1)
test(2**0 == 1)
test(1**2 == 1)
test(2**2 == 4)
test(10**3 == 1000)

test(1+2 == 3)
test(-2+2 == 0)
test(-2+-2 == -4)

test(1*2 == 2)
test(2*2 == 4)
test(2*-2 == -4)
test(-2*-2 == 4)

test(1/2 == 0.5)
test(-1/2 == -0.5)
test(1/-2 == -0.5)

test(2%2 == 0)
test(1%2 == 1)
test(-1%2 == -1)

test(1+2*3 == 7)
test((1+2)*3 == 9)

test(1<<1 == 2)
test(1<<8 == 256)
test(1<<31 == 0x80000000)
test(1<<32 == 1)

test(1>>1 == 0)
test(2>>1 == 1)
test(0x7fffffff>>30 == 1)
test(0x7fffffff>>31 == 0)
test(0x80000000>>31 == -1)

test(1>>>1 == 0)
test(2>>>1 == 1)
test(0x7fffffff>>>30 == 1)
test(0x7fffffff>>>31 == 0)
test(0x80000000>>>31 == 1)
test(0x80000000>>>32 == 0x80000000)

test(0<1)
test(1<2)
test(!(1<1))
test(1>0)
test(2>1)
test(!(1>1))
test(0<=1)
test(1<=2)
test(1<=1)
test(1>=0)
test(2>=1)
test(1>=1)

test(1<2<3)
test(!(1<2<2))
test(!(2<2<3))
test(1<2<=2)
test(2<=2<3)

test(1==2-1)
test(""!=null)
test(""=="")
test("a"=="a")
test("0"!=0)

test((1&0) == 0)
test((1&1) == 1)
test((0xffffffff & 0xf0f0f0f0) == 0xf0f0f0f0)

test((1^1) == 0)
test((1^0) == 1)
test((0^1) == 1)
test((0^0) == 0)
test((0xffffffff ^ 0xf0f0f0f0) == 0x0f0f0f0f)

test((1|1) == 1)
test((0|1) == 1)
test((1|0) == 1)
test((0|0) == 0)
test((0x0f0f0f0f | 0xf0f0f0f0) == 0xffffffff)

test(("a"||"b") == "a")
test((""||"b") == "b")

test(("a"&&"b") == "b")
test((""&&"b") == "")

a=b=c=12345
test(a==12345)
test(b==12345)
test(c==12345)

a=1
a+=1
test(a==2)
a-=1
test(a==1)
a+=1
a*=2
test(a==4)
a/=2
test(a==2)
a=5
a%=2
test(a==1)
a<<=1
test(a==2)
a>>=1
test(a==1)
a=0x80000000
a>>=1
test(a==-1073741824)
a=0x80000000
a>>>=1
test(a==1073741824)
a=10
a**=3
test(a==1000)
a="a"
a&&="b"
test(a=="b")
a=""
a&&="b"
test(a=="")
a="a"
a||="b"
test(a=="a")
a=""
a||="b"
test(a=="b")

a=1
v=(a+=1)
test(a==2)
test(v==2)
a=1
v=(a.+=1)
test(a==2)
test(v==1)

a=1
v=(a-=1)
test(a==0)
test(v==0)
a=1
v=(a.-=1)
test(a==0)
test(v==1)

a=2
v=(a*=2)
test(a==4)
test(v==4)
a=2
v=(a.*=2)
test(a==4)
test(v==2)

a=2
v=(a/=2)
test(a==1)
test(v==1)
a=2
v=(a./=2)
test(a==1)
test(v==2)

a=5
v=(a%=2)
test(a==1)
test(v==1)
a=5
v=(a.%=2)
test(a==1)
test(v==5)

a=1
v=(a<<=1)
test(a==2)
test(v==2)
a=1
v=(a.<<=1)
test(a==2)
test(v==1)

a=2
v=(a>>=1)
test(a==1)
test(v==1)
a=2
v=(a.>>=1)
test(a==1)
test(v==2)

test(1.(-$)==-1)
test([1,2,3][2]==3)
test({a:1}["a"]==1)

a={b:{c:1}}
test(a.b.c==1)

a=null
test(a?.b.c==null)

a={b:{}}
test(a.b?.c==null)

a=func(){
    return 1
}
test(a()==1)
a=null
test(a?()==null)
test(a?().b.c[1]==null)

a=[1,2,3,4,5]
test(a[1]==2)
a=null
test(a?[1]==null)
