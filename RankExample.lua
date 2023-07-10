--[[
  RankCore.New(compareNum, compareSet1, compareSet2)   --compareNum:参与比较数据数量 compareSet n:选择这项数据升序(0)降序(1)  return RankObj
  RankObj:Update(uid, data1, data2)  --更新或者插入一条数据 
  RankObj:Sort()  --手动排序
  RankObj:Length()  --排行榜内元素个数
  RankObj:QueryPos(uid)  --通过uid查询当前在排行榜的排名,返回排名
  RankObj:GetElementByUid(uid)  --通过uid查询当前在排行榜的数据，两个返回值，第一个是当前排名，第二个是数据table
  RankObj:GetElementByPos(pos)  --两个返回值，第一个是uid，第二个是数据table
  RankObj:ForeachByRange(left, right, callBack) --遍历排行榜区间为[left,right]的元素执行callBack,callBack原型为 function(uid, data1, data2) ,返回true提前结束循环
]]

local RankCore = require "RankCore"
-- 测试RankCore的接口
local function testRankCoreInterface()
    -- 创建排行榜对象，并添加数据
    local rankObj = RankCore.New(2, 1, 0)
    rankObj:Update(1, 100, 200)
    rankObj:Update(2, 200, 300)
    rankObj:Update(3, 150, 250)
    rankObj:Update(4, 250, 150)

    -- 测试Length()
    assert(rankObj:Length() == 4, "Length接口返回值错误！")

    -- 测试QueryPos()和GetDataByUid()
    local pos, data = rankObj:GetElementByUid(1)
    assert(pos == 4 and data[1] == 100 and data[2] == 200, "QueryPos或GetDataByUid接口返回值错误！")

    -- 测试ForeachByRange()
    -- local rangeData = {}
    -- rankObj:ForeachByRange(2, 4, function(uid, data1, data2) table.insert(rangeData, {uid = uid, data1 = data1, data2 = data2}) end)
    -- assert(#rangeData == 3 and rangeData[1].uid == 2 and rangeData[2].uid == 3 and rangeData[3].uid == 1, "ForeachByRange接口返回值错误！")

    -- 更新数据并手动排序
    rankObj:Update(1, 400, 500)
    rankObj:Sort()

    local uid, data = rankObj:GetElementByPos(1)
    print(uid, data[1], data[2])

    -- 再次测试QueryPos()和GetDataByUid()
    pos, data = rankObj:GetElementByUid(1)
    assert(pos == 1 and data[1] == 400 and data[2] == 500, "QueryPos或GetDataByUid接口返回值错误！")

    rankObj:ForeachByRange(1, 100, function(uid, pos, data) print(uid, pos, table.unpack(data)) end)
    rankObj:__close()
end

testRankCoreInterface()
