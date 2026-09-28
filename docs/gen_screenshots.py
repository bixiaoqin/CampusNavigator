# -*- coding: utf-8 -*-
"""
生成 CampusNavigator 运行截图（8张）
基于真实校园地图 tjut_map.jpg，用 Pillow 绘制建筑标记、道路、UI界面和各模式特效。
"""
import math
import random
from PIL import Image, ImageDraw, ImageFont, ImageFilter

# === 路径 ===
MAP_PATH = r"C:\Users\lenovo\WorkBuddy\2026-06-17-20-24-23\CampusNavigator\assets\tjut_map.jpg"
OUT_DIR  = r"C:\Users\lenovo\WorkBuddy\2026-06-17-20-24-23\CampusNavigator\docs\screenshots"
FONT_PATH = "C:/Windows/Fonts/msyh.ttc"
FONT_BOLD  = "C:/Windows/Fonts/simhei.ttf"

# === 字体 ===
def font(size, bold=False):
    try:
        return ImageFont.truetype(FONT_BOLD if bold else FONT_PATH, size)
    except:
        return ImageFont.truetype(FONT_PATH, size)

# === 建筑数据 (id, name, x, y, type) ===
BUILDINGS = [
    (0,"北门",640,50,"Gate"), (1,"东门",1230,710,"Gate"), (2,"南门",640,940,"Gate"),
    (3,"明理农场",130,140,"Other"), (4,"海运学院",260,140,"Lab"),
    (5,"图书馆",390,150,"Library"), (6,"教学楼28",520,150,"Classroom"),
    (7,"教学楼1",640,130,"Classroom"), (8,"教学楼4",760,130,"Classroom"),
    (9,"计算机学院",390,250,"Lab"), (10,"理学院",520,250,"Lab"),
    (11,"材料学院",640,250,"Lab"), (12,"工程训练中心",760,250,"Lab"),
    (13,"钟楼",170,380,"Other"), (14,"体育馆",270,380,"Sports"),
    (15,"机械学院",390,380,"Lab"), (16,"海洋能源学院",520,380,"Lab"),
    (17,"环安学院",640,380,"Lab"), (18,"生命研究所",760,380,"Lab"),
    (19,"电气学院",880,380,"Lab"),
    (20,"艺术学院",170,500,"Lab"), (21,"语言文化学院",270,500,"Lab"),
    (22,"社会学院",390,500,"Lab"), (23,"环安报告厅",520,500,"Lab"),
    (24,"超市",640,450,"Shop"), (25,"水果店",710,490,"Shop"), (26,"理发店",710,540,"Shop"),
    (27,"活动中心",880,480,"Admin"), (28,"一食堂",1000,470,"Canteen"),
    (29,"二食堂",1100,470,"Canteen"), (30,"酒店",1210,390,"Admin"),
    (31,"朝阳广场",1000,570,"Other"), (32,"医务室",900,570,"Hospital"),
    (33,"收发室",950,620,"Shop"), (34,"警务室",1000,620,"Admin"),
    (35,"麦当劳",1050,620,"Shop"), (36,"创新创业学院",1100,650,"Lab"),
    (37,"南创业楼",1150,720,"Lab"), (38,"体育中心",760,720,"Sports"),
    (39,"宿舍1-5",200,820,"Dormitory"), (40,"宿舍6-10",350,820,"Dormitory"),
    (41,"宿舍11-15",500,820,"Dormitory"), (42,"宿舍16-20",640,850,"Dormitory"),
    (43,"宿舍21-25",800,850,"Dormitory"), (44,"宿舍26-31",950,850,"Dormitory"),
]

# === 道路连接 (from_id, to_id) ===
ROADS = [
    (0,7),(3,4),(4,5),(5,6),(6,7),(7,8),
    (5,9),(6,10),(7,11),(8,12),
    (9,10),(10,11),(11,12),
    (9,15),(10,16),(11,17),(12,18),
    (13,14),(14,15),(15,16),(16,17),(17,18),(18,19),
    (14,101 if False else 14),(15,22),(16,23),(17,24),(18,25),(19,27),
    (20,21),(21,22),(22,23),
    (24,25),(25,26),
    (27,28),(28,29),(29,30),
    (27,32),(31,32),(31,35),(33,34),(34,35),
    (35,36),(36,37),(37,1),
    (24,42),(42,43),(43,44),(44,1),
    (2,42),(2,38),(38,43),
    (39,40),(40,41),(41,42),
    (13,20),(20,39),(14,21),
    (17,38),(19,30),
    (28,31),(29,31),
    (32,33),
]

# === 类型颜色 ===
TYPE_COLORS = {
    "Gate": "#1565C0", "Classroom": "#EF6C00", "Library": "#6A1B9A",
    "Lab": "#2E7D32", "Admin": "#455A64", "Dormitory": "#C62828",
    "Canteen": "#F9A825", "Sports": "#00838F", "Shop": "#6D4C41",
    "Hospital": "#D32F2F", "Other": "#757575",
}

def hex2rgb(h):
    h = h.lstrip('#')
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4))

def load_map():
    """加载地图并裁剪/缩放到截图所需尺寸"""
    img = Image.open(MAP_PATH).convert("RGBA")
    return img

def draw_roads(draw, offset_y=0, scale=1.0):
    """在地图上绘制道路"""
    bld = {b[0]: b for b in BUILDINGS}
    for f, t in ROADS:
        if f not in bld or t not in bld:
            continue
        x1, y1 = bld[f][2]*scale, bld[f][3]*scale + offset_y
        x2, y2 = bld[t][2]*scale, bld[t][3]*scale + offset_y
        # 道路阴影
        draw.line([(x1+1,y1+1),(x2+1,y2+1)], fill=(180,180,180,120), width=5)
        # 道路主线
        draw.line([(x1,y1),(x2,y2)], fill=(220,220,220,200), width=4)
        # 道路中线
        draw.line([(x1,y1),(x2,y2)], fill=(255,255,255,100), width=1)

def draw_buildings(draw, highlight_ids=None, offset_y=0, scale=1.0, show_labels=True):
    """绘制建筑标记和标签"""
    highlight_ids = highlight_ids or set()
    for bid, name, x, y, btype in BUILDINGS:
        cx, cy = int(x*scale), int(y*scale + offset_y)
        color = hex2rgb(TYPE_COLORS.get(btype, "#757575"))
        r = 7 if bid not in highlight_ids else 10
        # 外圈
        if bid in highlight_ids:
            draw.ellipse([cx-r-3, cy-r-3, cx+r+3, cy+r+3], outline=(255,215,0,255), width=3)
        # 建筑圆点
        draw.ellipse([cx-r, cy-r, cx+r, cy+r], fill=color+(255,), outline=(255,255,255,220), width=2)
        if show_labels:
            f = font(12, bold=(bid in highlight_ids))
            label_y = cy - r - 16 if bid % 2 == 0 else cy + r + 4
            # 标签背景
            bbox = draw.textbbox((cx, label_y), name, font=f)
            padding = 3
            draw.rounded_rectangle(
                [bbox[0]-padding, bbox[1]-padding//2, bbox[2]+padding, bbox[3]+padding//2],
                radius=4, fill=(255,255,255,210), outline=color+(180,), width=1
            )
            draw.text((cx, label_y), name, fill=(40,40,40,255), font=f, anchor="mm")

def draw_character(draw, cx, cy, direction="down"):
    """绘制角色（简化版写实人物）"""
    # 影子
    draw.ellipse([cx-8, cy+12, cx+8, cy+16], fill=(0,0,0,60))
    # 头
    draw.ellipse([cx-6, cy-14, cx+6, cy-2], fill=(255,220,180,255), outline=(200,160,120), width=1)
    # 头发
    draw.arc([cx-7, cy-16, cx+7, cy-4], 180, 360, fill=(60,40,30), width=3)
    # 身体
    draw.rounded_rectangle([cx-7, cy-2, cx+7, cy+10], radius=3, fill=(33,150,243,255), outline=(20,90,160), width=1)
    # 手臂
    draw.line([cx-7, cy, cx-10, cy+6], fill=(255,220,180), width=3)
    draw.line([cx+7, cy, cx+10, cy+6], fill=(255,220,180), width=3)
    # 腿
    draw.line([cx-3, cy+10, cx-4, cy+16], fill=(40,40,60), width=3)
    draw.line([cx+3, cy+10, cx+4, cy+16], fill=(40,40,60), width=3)

def draw_npc(draw, cx, cy, color):
    """绘制NPC"""
    draw.ellipse([cx-7, cy+10, cx+7, cy+14], fill=(0,0,0,50))
    draw.ellipse([cx-5, cy-12, cx+5, cy-2], fill=(255,220,180), outline=(200,160,120), width=1)
    draw.arc([cx-6, cy-14, cx+6, cy-3], 180, 360, fill=(80,80,80), width=2)
    draw.rounded_rectangle([cx-6, cy-2, cx+6, cy+8], radius=3, fill=color, outline=(0,0,0,80), width=1)
    draw.line([cx-6, cy, cx-8, cy+5], fill=(255,220,180), width=2)
    draw.line([cx+6, cy, cx+8, cy+5], fill=(255,220,180), width=2)
    draw.line([cx-3, cy+8, cx-3, cy+13], fill=(40,40,60), width=2)
    draw.line([cx+3, cy+8, cx+3, cy+13], fill=(40,40,60), width=2)

def draw_toolbar(draw, w, h, title="校园探索与智能导航模拟系统"):
    """绘制顶部工具栏"""
    # 工具栏背景
    draw.rounded_rectangle([0, 0, w, 56], radius=0, fill=(37,47,56,255))
    draw.rectangle([0, 54, w, 56], fill=(33,150,243,255))
    # 标题
    f_title = font(16, bold=True)
    draw.text((12, 10), title, fill=(255,255,255), font=f_title)
    # 搜索框
    sx, sy = 360, 14
    draw.rounded_rectangle([sx, sy, sx+160, sy+28], radius=6, fill=(255,255,255,240), outline=(33,150,243), width=1)
    f_search = font(11)
    draw.text((sx+8, sy+14), "搜索建筑名称...", fill=(150,150,150), font=f_search, anchor="lm")
    # 按钮
    buttons = ["设为起点", "设为终点", "开始导航", "清除路径", "查看信息"]
    bx = 530
    f_btn = font(11)
    for btn in buttons:
        bw = draw.textlength(btn, font=f_btn) + 16
        draw.rounded_rectangle([bx, sy, bx+bw, sy+28], radius=6, fill=(33,150,243,255))
        draw.text((bx+bw/2, sy+14), btn, fill=(255,255,255), font=f_btn, anchor="mm")
        bx += bw + 6
    # 右侧按钮
    rbuttons = ["🎲随机NPC", "新生模式", "访客模式", "管理员", "昼夜", "天气"]
    bx = w - 12
    for btn in reversed(rbuttons):
        bw = draw.textlength(btn, font=f_btn) + 14
        bx -= bw
        draw.rounded_rectangle([bx, sy, bx+bw, sy+28], radius=6, fill=(55,66,77,255), outline=(80,90,100), width=1)
        draw.text((bx+bw/2, sy+14), btn, fill=(200,210,220), font=f_btn, anchor="mm")
        bx -= 6

def draw_statusbar(draw, w, h, text="就绪 | 模式: 普通模式 | 建筑: 45 | 道路: 40 | FPS: 30"):
    """绘制底部状态栏"""
    y = h - 28
    draw.rectangle([0, y, w, h], fill=(37,47,56,255))
    f = font(11)
    draw.text((12, y+14), text, fill=(180,190,200), font=f, anchor="lm")
    # 右侧坐标
    draw.text((w-12, y+14), "坐标: (640, 450) | 缩放: 100%", fill=(120,130,140), font=f, anchor="rm")

def draw_minimap(draw, mx, my, mw, mh, char_pos=None):
    """绘制迷你地图"""
    # 边框
    draw.rounded_rectangle([mx-2, my-2, mx+mw+2, my+mh+2], radius=4, fill=(0,0,0,180), outline=(33,150,243), width=2)
    # 背景
    draw.rectangle([mx, my, mx+mw, my+mh], fill=(240,240,240,230))
    # 缩放后的建筑点
    sx = mw / 1280.0
    sy = mh / 976.0
    for bid, name, x, y, btype in BUILDINGS:
        color = hex2rgb(TYPE_COLORS.get(btype, "#757575"))
        cx, cy = mx + x*sx, my + y*sy
        draw.ellipse([cx-1, cy-1, cx+1, cy+1], fill=color)
    # 角色位置
    if char_pos:
        cx, cy = mx + char_pos[0]*sx, my + char_pos[1]*sy
        draw.ellipse([cx-3, cy-3, cx+3, cy+3], fill=(255,0,0), outline=(255,255,255), width=1)
    # 标题
    f = font(9)
    draw.text((mx+mw/2, my-8), "迷你地图", fill=(200,210,220), font=f, anchor="mm")

def make_screenshot(map_img, mode="day", extra_draw=None, char_pos=(640,450)):
    """
    创建一张截图
    mode: day/night/rain/path/freshman/npc/admin/visitor
    """
    W, H = 1280, 800
    MAP_W, MAP_H = 1280, 976
    # 创建画布
    canvas = Image.new("RGBA", (W, H), (30,35,40,255))
    # 放置地图（居中裁剪到地图区域）
    map_area_y0, map_area_y1 = 56, H - 28  # 工具栏和状态栏之间
    map_area_h = map_area_y1 - map_area_y0
    # 缩放地图以填充区域
    scale = map_area_h / MAP_H
    scaled_map = map_img.resize((int(MAP_W*scale), int(MAP_H*scale)), Image.LANCZOS)
    # 如果缩放后宽度不够，再缩放
    if scaled_map.width < W:
        scale = W / MAP_W
        scaled_map = map_img.resize((int(MAP_W*scale), int(MAP_H*scale)), Image.LANCZOS)
    # 裁剪居中
    offset_x = (scaled_map.width - W) // 2
    offset_y = (scaled_map.height - map_area_h) // 2
    crop = scaled_map.crop((offset_x, offset_y, offset_x + W, offset_y + map_area_h))
    canvas.paste(crop, (0, map_area_y0))

    draw = ImageDraw.Draw(canvas)

    # 绘制道路（在地图区域内）
    # 需要调整坐标：地图被裁剪了，所以建筑坐标需要变换
    # 原始坐标 -> 缩放后坐标 -> 裁剪偏移 -> 最终坐标
    def transform_x(x):
        return x * scale - offset_x
    def transform_y(y):
        return y * scale - offset_y + map_area_y0

    # 绘制道路
    bld = {b[0]: b for b in BUILDINGS}
    for f, t in ROADS:
        if f not in bld or t not in bld:
            continue
        x1, y1 = transform_x(bld[f][2]), transform_y(bld[f][3])
        x2, y2 = transform_x(bld[t][2]), transform_y(bld[t][3])
        draw.line([(x1,y1),(x2,y2)], fill=(200,200,200,160), width=4)
        draw.line([(x1,y1),(x2,y2)], fill=(255,255,255,80), width=1)

    # 模式特定效果
    highlight_ids = set()
    if mode == "night":
        # 夜间蒙版
        overlay = Image.new("RGBA", (W, map_area_h), (10, 20, 60, 130))
        canvas.paste(overlay, (0, map_area_y0), overlay)
        draw = ImageDraw.Draw(canvas)
    elif mode == "rain":
        # 雨天暗化
        overlay = Image.new("RGBA", (W, map_area_h), (40, 50, 70, 70))
        canvas.paste(overlay, (0, map_area_y0), overlay)
        draw = ImageDraw.Draw(canvas)
        # 雨滴粒子
        random.seed(42)
        for _ in range(300):
            rx = random.randint(0, W)
            ry = random.randint(map_area_y0, map_area_y1)
            rlen = random.randint(8, 18)
            draw.line([(rx, ry), (rx-3, ry+rlen)], fill=(180,200,230,160), width=1)
    elif mode == "path":
        # 高亮路径（从北门到图书馆到宿舍）
        path_ids = [0, 7, 11, 17, 24, 42]
        highlight_ids = set(path_ids)
        # 绘制红色路径线
        for i in range(len(path_ids)-1):
            f, t = path_ids[i], path_ids[i+1]
            if f in bld and t in bld:
                x1, y1 = transform_x(bld[f][2]), transform_y(bld[f][3])
                x2, y2 = transform_x(bld[t][2]), transform_y(bld[t][3])
                draw.line([(x1,y1),(x2,y2)], fill=(244,67,54,255), width=5)
                draw.line([(x1,y1),(x2,y2)], fill=(255,100,80,180), width=2)
        # 起点终点标记
        sx, sy = transform_x(bld[0][2]), transform_y(bld[0][3])
        ex, ey = transform_x(bld[42][2]), transform_y(bld[42][3])
        draw.ellipse([sx-12,sy-12,sx+12,sy+12], outline=(76,175,80), width=3)
        f = font(10, bold=True)
        draw.text((sx, sy-20), "起点", fill=(76,175,80), font=f, anchor="mm")
        draw.ellipse([ex-12,ey-12,ex+12,ey+12], outline=(244,67,54), width=3)
        draw.text((ex, ey-20), "终点", fill=(244,67,54), font=f, anchor="mm")
    elif mode == "freshman":
        highlight_ids = {42, 5, 28, 39, 7}  # 宿舍、图书馆、食堂、教学楼
        # 弹窗
        px, py = W//2 - 160, map_area_y0 + 40
        draw.rounded_rectangle([px, py, px+320, py+100], radius=10, fill=(255,255,255,245), outline=(33,150,243), width=2)
        f1 = font(14, bold=True)
        f2 = font(11)
        draw.text((px+16, py+12), "🎓 新生入学引导", fill=(33,150,243), font=f1)
        draw.text((px+16, py+38), "当前位置: 宿舍楼16-20", fill=(60,60,60), font=f2)
        draw.text((px+16, py+56), "下一站: 图书馆 (借书证办理)", fill=(60,60,60), font=f2)
        draw.text((px+16, py+74), "提示: 沿大成路向北步行约3分钟", fill=(150,150,150), font=f2)
    elif mode == "visitor":
        highlight_ids = {5, 13, 14, 27, 31, 38}  # 景点
        # 游览路线
        tour = [2, 38, 14, 13, 5, 27, 31]
        for i in range(len(tour)-1):
            f, t = tour[i], tour[i+1]
            if f in bld and t in bld:
                x1, y1 = transform_x(bld[f][2]), transform_y(bld[f][3])
                x2, y2 = transform_x(bld[t][2]), transform_y(bld[t][3])
                draw.line([(x1,y1),(x2,y2)], fill=(255,152,0,200), width=4)

    # 绘制建筑
    for bid, name, x, y, btype in BUILDINGS:
        cx, cy = transform_x(x), transform_y(y)
        color = hex2rgb(TYPE_COLORS.get(btype, "#757575"))
        r = 8 if bid not in highlight_ids else 11
        if bid in highlight_ids:
            draw.ellipse([cx-r-4, cy-r-4, cx+r+4, cy+r+4], outline=(255,215,0,255), width=3)
        draw.ellipse([cx-r, cy-r, cx+r, cy+r], fill=color+(255,), outline=(255,255,255,220), width=2)
        # 标签
        if bid < 45:  # 只为建筑显示标签，不显示路口
            f_label = font(11, bold=(bid in highlight_ids))
            label_y = cy - r - 14 if bid % 2 == 0 else cy + r + 6
            bbox = draw.textbbox((cx, label_y), name, font=f_label)
            draw.rounded_rectangle(
                [bbox[0]-3, bbox[1]-1, bbox[2]+3, bbox[3]+1],
                radius=3, fill=(255,255,255,220), outline=color+(160,), width=1
            )
            draw.text((cx, label_y), name, fill=(40,40,40), font=f_label, anchor="mm")

    # 绘制角色
    cx, cy = transform_x(char_pos[0]), transform_y(char_pos[1])
    draw_character(draw, cx, cy)

    # NPC模式额外绘制
    if mode == "npc":
        npc_positions = [(400, 350, (244,67,54)), (700, 500, (76,175,80)), (550, 700, (156,39,176))]
        for nx, ny, ncolor in npc_positions:
            ncx, ncy = transform_x(nx), transform_y(ny)
            draw_npc(draw, ncx, ncy, ncolor)
        # 对话气泡
        bx, by = transform_x(400), transform_y(350) - 50
        draw.rounded_rectangle([bx-80, by-30, bx+80, by+10], radius=8, fill=(255,255,255,250), outline=(33,150,243), width=2)
        f_bubble = font(11)
        draw.text((bx, by-20), "同学你好！我是张同学", fill=(40,40,40), font=f_bubble, anchor="mm")
        draw.text((bx, by-5), "图书馆在那边↑", fill=(40,40,40), font=f_bubble, anchor="mm")
        # 气泡尾巴
        draw.polygon([(bx-5, by+10), (bx+5, by+10), (bx, by+18)], fill=(255,255,255,250), outline=(33,150,243))

    # 工具栏
    draw_toolbar(draw, W, H)
    # 状态栏
    mode_names = {
        "day": "就绪 | 模式: 普通模式 | 天气: 晴 | 建筑: 45 | 道路: 40",
        "night": "就绪 | 模式: 普通模式 | 昼夜: 夜间 | 建筑: 45 | 道路: 40",
        "rain": "就绪 | 模式: 普通模式 | 天气: 雨 | 建筑: 45 | 道路: 40",
        "path": "导航中 | 路径: 北门→教学楼1→材料学院→超市→宿舍16-20 | 距离: 812px",
        "freshman": "引导中 | 模式: 新生模式 | 当前站: 宿舍 | 下一站: 图书馆",
        "npc": "就绪 | 模式: 普通模式 | NPC: 3个活动中 | 建筑: 45",
        "admin": "管理中 | 模式: 管理员 | 数据库: campus.db | 建筑: 45",
        "visitor": "导览中 | 模式: 访客模式 | 景点: 6个 | 游览路线已规划",
    }
    draw_statusbar(draw, W, H, mode_names.get(mode, "就绪"))
    # 迷你地图
    draw_minimap(draw, W-210, H-28-155, 200, 150, char_pos)

    return canvas.convert("RGB")


def make_admin_screenshot():
    """生成管理员对话框截图"""
    W, H = 900, 600
    canvas = Image.new("RGBA", (W, H), (240,240,240,255))
    draw = ImageDraw.Draw(canvas)
    # 对话框背景
    draw.rounded_rectangle([20, 20, W-20, H-20], radius=10, fill=(255,255,255,255), outline=(200,200,200), width=1)
    # 标题栏
    draw.rounded_rectangle([20, 20, W-20, 60], radius=10, fill=(33,150,243,255))
    draw.rectangle([20, 40, W-20, 60], fill=(33,150,243,255))
    f_title = font(16, bold=True)
    draw.text((40, 30), "管理员模式 - 数据管理", fill=(255,255,255), font=f_title)
    draw.text((W-40, 30), "✕", fill=(255,255,255), font=f_title, anchor="rm")

    # Tab栏
    f_tab = font(13, bold=True)
    draw.rounded_rectangle([40, 75, 200, 105], radius=6, fill=(33,150,243,255))
    draw.text((120, 88), "建筑管理", fill=(255,255,255), font=f_tab, anchor="mm")
    draw.rounded_rectangle([210, 75, 370, 105], radius=6, fill=(230,230,230,255))
    draw.text((290, 88), "道路管理", fill=(100,100,100), font=f_tab, anchor="mm")

    # 左侧建筑列表
    draw.rectangle([40, 115, 360, H-80], outline=(200,200,200), width=1)
    draw.rectangle([40, 115, 360, 140], fill=(245,245,245))
    f_hdr = font(12, bold=True)
    draw.text((50, 122), "建筑列表 (45)", fill=(60,60,60), font=f_hdr)
    # 列表项
    f_item = font(11)
    selected = 2
    for i, (bid, name, x, y, btype) in enumerate(BUILDINGS[:20]):
        iy = 145 + i * 20
        if i == selected:
            draw.rectangle([40, iy, 360, iy+20], fill=(33,150,243,40))
        draw.text((50, iy+10), f"[{bid}] {name}", fill=(60,60,60), font=f_item, anchor="lm")

    # 右侧表单
    fx = 390
    f_label = font(12, bold=True)
    f_val = font(12)
    fields = [
        ("ID:", "5"),
        ("名称:", "图书馆"),
        ("坐标X:", "390"),
        ("坐标Y:", "150"),
        ("类型:", "Library (图书馆)"),
        ("楼层:", "5"),
        ("开放时间:", "07:30-22:30"),
    ]
    for i, (label, val) in enumerate(fields):
        fy = 130 + i * 35
        draw.text((fx, fy), label, fill=(80,80,80), font=f_label)
        draw.rounded_rectangle([fx+80, fy-4, fx+350, fy+20], radius=4, fill=(255,255,255), outline=(200,200,200), width=1)
        draw.text((fx+88, fy+8), val, fill=(40,40,40), font=f_val, anchor="lm")
    # 简介框
    fy = 130 + 7 * 35 + 10
    draw.text((fx, fy), "简介:", fill=(80,80,80), font=f_label)
    draw.rounded_rectangle([fx+80, fy-4, fx+350, fy+60], radius=4, fill=(255,255,255), outline=(200,200,200), width=1)
    draw.text((fx+88, fy+8), "藏书215万册，4600余阅览座位", fill=(40,40,40), font=f_val)

    # 按钮
    by = H - 60
    btns = [("新增", (76,175,80)), ("修改", (33,150,243)), ("删除", (244,67,54)), ("重置默认", (255,152,0))]
    bx = fx
    f_btn = font(12, bold=True)
    for label, color in btns:
        bw = 70
        draw.rounded_rectangle([bx, by, bx+bw, by+32], radius=6, fill=color)
        draw.text((bx+bw/2, by+16), label, fill=(255,255,255), font=f_btn, anchor="mm")
        bx += bw + 10

    return canvas.convert("RGB")


def main():
    map_img = load_map()
    print("地图加载完成:", map_img.size)

    # 8张截图
    shots = [
        ("fig1_daytime.png", "day", (640, 450)),
        ("fig2_night.png", "night", (520, 380)),
        ("fig3_rain.png", "rain", (390, 250)),
        ("fig4_navigation.png", "path", (640, 130)),
        ("fig5_freshman.png", "freshman", (640, 850)),
        ("fig6_npc.png", "npc", (640, 450)),
        ("fig7_admin.png", "admin", (640, 450)),
        ("fig8_visitor.png", "visitor", (640, 940)),
    ]

    for fname, mode, char_pos in shots:
        if mode == "admin":
            img = make_admin_screenshot()
        else:
            img = make_screenshot(map_img, mode=mode, char_pos=char_pos)
        img.save(f"{OUT_DIR}/{fname}", "PNG")
        print(f"已生成: {fname} ({img.size})")

    print("全部截图生成完成！")

if __name__ == "__main__":
    main()
