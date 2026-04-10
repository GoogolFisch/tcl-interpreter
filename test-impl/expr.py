

class tee:
    left = None
    right = None
    data = ""
    def __init__(self,r):
        self.data = r

    def __str__(self):
        return f"{self.data} ({self.left},{self.right})"

def isDigit(x):
    return x in "0123456789"
def isNum(x):
    if(len(x) == 0):return True
    if(x[0] == '-'): x = x[1:]
    st = 0
    for ch in x:
        if(ch == '.' and st == 0):st = 1
        if(ch not in "0123456789"): return False
    return True
def isVar(x):
    if(len(x) == 0):return True
    if(x[0] == '$'):x = x[1:]
    if(len(x) == 0):return True
    return x.isalpha()
def isFitting(x,y):
    if(y in ' \n\r\t'):return False
    if(isNum(x) and isDigit(y)):return True
    if(isNum(x) and '.' not in x and y == '.'):return True
    if(isVar(x) and y.isalpha()):return True
    if(x.isalpha() and y.isalpha()):return True
    return False


def tokenise(x):
    o = [tee(x[0])]
    x = x[1:]
    state = 0
    for ch in x:
        if(isFitting(o[-1].data,ch)):
            o[-1].data += ch
        elif(ch in ' \n\r\t'):pass
        else:
           o.append(tee(ch))
    return o

def opAct(ls,idx,low,upp):
    lidx = idx - 1
    ridx = idx + 1
    while(lidx >= low and ls[lidx] == None):
        lidx -= 1
    while(ridx < upp and ls[ridx] == None):
        ridx += 1
    if(lidx < low or ridx >= upp):
        print("error during parsing!")
        print([str(t) for t in tok])
        print(idx,low,upp)
        raise Exception("Error")
    left = ls[lidx]
    right = ls[ridx]
    #
    me = ls[idx]
    me.left = left
    me.right = right
    #
    ls[lidx] = None
    ls[ridx] = None

def treeage(ls,low,upp):
    stack = 0
    prev = -1
    for idx,item in enumerate(ls):
        if(item is None):continue
        if(item.data == '('):
            if(stack == 0):
                prev = idx
            stack += 1
        if(item.data == ')'):
            stack -= 1
            if(stack == 0):
                ls[prev] = None
                ls[idx] = None
                treeage(ls,prev,idx)
    # =======================
    # combine * / %
    lidx = 0
    ridx = 0
    for idx in range(low,upp):
        item = ls[idx]
        if(item is None):continue
        if(item.data in '*/%'):
            opAct(ls,idx,low,upp)
    for idx in range(low,upp):
        item = ls[idx]
        if(item is None):continue
        if(item.data in '+-'):
            opAct(ls,idx,low,upp)




s = input(">>")
tok = tokenise(s)
print([str(t) for t in tok])
treeage(tok,0,len(tok))
print([str(t) for t in tok])
