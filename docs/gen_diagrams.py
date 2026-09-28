# -*- coding: utf-8 -*-
"""
生成 UML 类图、用例图、架构图、算法可视化图（SVG → PNG）
"""
import cairosvg
import os
import re

OUT_DIR = r"C:\Users\lenovo\WorkBuddy\2026-06-17-20-24-23\CampusNavigator\docs\diagrams"

# ============================================================
#  UML 类图
# ============================================================
UML_CLASS_DIAGRAM = r'''
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1800 1400" font-family="Microsoft YaHei,SimHei,sans-serif">
  <defs>
    <!-- 继承箭头（空心三角） -->
    <marker id="inherit" viewBox="0 0 12 12" refX="11" refY="6" markerWidth="14" markerHeight="14" orient="auto">
      <path d="M0,0 L11,6 L0,12 Z" fill="white" stroke="#37474F" stroke-width="1.5"/>
    </marker>
    <!-- 组合（实心菱形） -->
    <marker id="compose" viewBox="0 0 12 12" refX="1" refY="6" markerWidth="14" markerHeight="14" orient="auto">
      <path d="M0,6 L6,0 L12,6 L6,12 Z" fill="#37474F" stroke="#37474F" stroke-width="1"/>
    </marker>
    <!-- 聚合（空心菱形） -->
    <marker id="aggregate" viewBox="0 0 12 12" refX="1" refY="6" markerWidth="14" markerHeight="14" orient="auto">
      <path d="M0,6 L6,0 L12,6 L6,12 Z" fill="white" stroke="#37474F" stroke-width="1"/>
    </marker>
    <!-- 关联（实线箭头） -->
    <marker id="assoc" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="10" markerHeight="10" orient="auto">
      <path d="M0,0 L9,5 L0,10" fill="none" stroke="#37474F" stroke-width="1.5"/>
    </marker>
    <!-- 依赖（虚线箭头） -->
    <marker id="depend" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="10" markerHeight="10" orient="auto">
      <path d="M0,0 L9,5 L0,10" fill="none" stroke="#78909C" stroke-width="1.5"/>
    </marker>
    <style>
      .cls-box { fill: #FFFFFF; stroke: #37474F; stroke-width: 1.5; }
      .cls-name { font-size: 15px; font-weight: bold; text-anchor: middle; }
      .cls-attr { font-size: 11px; }
      .cls-method { font-size: 11px; }
      .cls-sep { stroke: #37474F; stroke-width: 1; }
      .layer-label { font-size: 13px; font-weight: bold; fill: #78909C; }
      .qt-box { fill: #E3F2FD; stroke: #1565C0; stroke-width: 1.5; }
      .qt-name { font-size: 14px; font-weight: bold; text-anchor: middle; fill: #1565C0; }
      .rel-label { font-size: 10px; fill: #546E7A; }
      .legend-box { fill: #FAFAFA; stroke: #BDBDBD; stroke-width: 1; }
    </style>
  </defs>

  <!-- 背景 -->
  <rect width="1800" height="1400" fill="#FAFAFA"/>

  <!-- 标题 -->
  <text x="900" y="30" font-size="20" font-weight="bold" text-anchor="middle" fill="#263238">CampusNavigator 系统 UML 类图</text>

  <!-- 层标签 -->
  <text x="200" y="70" class="layer-label">视图层 (View)</text>
  <text x="900" y="70" class="layer-label">数据层 (Data)</text>
  <text x="1400" y="70" class="layer-label">模型层 (Model)</text>

  <!-- ============ Qt 基类（顶部）============ -->
  <rect x="60" y="85" width="180" height="50" class="qt-box" rx="4"/>
  <text x="150" y="108" class="qt-name">QMainWindow</text>
  <text x="150" y="125" class="cls-attr" text-anchor="middle" fill="#1565C0">«Qt 基类»</text>

  <rect x="300" y="85" width="180" height="50" class="qt-box" rx="4"/>
  <text x="390" y="108" class="qt-name">QGraphicsScene</text>
  <text x="390" y="125" class="cls-attr" text-anchor="middle" fill="#1565C0">«Qt 基类»</text>

  <rect x="540" y="85" width="180" height="50" class="qt-box" rx="4"/>
  <text x="630" y="108" class="qt-name">QGraphicsObject</text>
  <text x="630" y="125" class="cls-attr" text-anchor="middle" fill="#1565C0">«Qt 基类»</text>

  <rect x="780" y="85" width="160" height="50" class="qt-box" rx="4"/>
  <text x="860" y="108" class="qt-name">QDialog</text>
  <text x="860" y="125" class="cls-attr" text-anchor="middle" fill="#1565C0">«Qt 基类»</text>

  <rect x="980" y="85" width="180" height="50" class="qt-box" rx="4"/>
  <text x="1070" y="108" class="qt-name">QGraphicsItem</text>
  <text x="1070" y="125" class="cls-attr" text-anchor="middle" fill="#1565C0">«Qt 基类»</text>

  <!-- ============ MainWindow ============ -->
  <rect x="40" y="200" width="320" height="320" class="cls-box" rx="4"/>
  <rect x="40" y="200" width="320" height="28" fill="#1565C0" rx="4"/>
  <rect x="40" y="220" width="320" height="8" fill="#1565C0"/>
  <text x="200" y="219" class="cls-name" fill="white">MainWindow</text>
  <text x="200" y="244" class="cls-attr" text-anchor="middle" fill="#78909C">«controller, Q_OBJECT»</text>
  <line x1="40" y1="252" x2="360" y2="252" class="cls-sep"/>
  <text x="50" y="268" class="cls-attr">- mapView_: QGraphicsView*</text>
  <text x="50" y="284" class="cls-attr">- miniMapView_: QGraphicsView*</text>
  <text x="50" y="300" class="cls-attr">- mapScene_: MapScene*</text>
  <text x="50" y="316" class="cls-attr">- character_: CharacterItem*</text>
  <text x="50" y="332" class="cls-attr">- weatherOverlay_: WeatherOverlay*</text>
  <text x="50" y="348" class="cls-attr">- campus_: Graph</text>
  <text x="50" y="364" class="cls-attr">- npcs_: vector&lt;NpcItem*&gt;</text>
  <text x="50" y="380" class="cls-attr">- moveTimer_: QTimer*</text>
  <text x="50" y="396" class="cls-attr">- navTimer_: QTimer*</text>
  <text x="50" y="412" class="cls-attr">- appMode_: AppMode</text>
  <line x1="40" y1="420" x2="360" y2="420" class="cls-sep"/>
  <text x="50" y="436" class="cls-method">+ setupUI()</text>
  <text x="50" y="452" class="cls-method">+ initMapData()</text>
  <text x="50" y="468" class="cls-method">+ onNavigate()</text>
  <text x="50" y="484" class="cls-method">+ onCharacterTimer()</text>
  <text x="50" y="500" class="cls-method">+ startAutoNavigate(path)</text>
  <text x="50" y="516" class="cls-method">+ onToggleDayNight()</text>

  <!-- ============ MapScene ============ -->
  <rect x="400" y="200" width="260" height="240" class="cls-box" rx="4"/>
  <rect x="400" y="200" width="260" height="28" fill="#2E7D32" rx="4"/>
  <rect x="400" y="220" width="260" height="8" fill="#2E7D32"/>
  <text x="530" y="219" class="cls-name" fill="white">MapScene</text>
  <text x="530" y="244" class="cls-attr" text-anchor="middle" fill="#78909C">«Q_OBJECT»</text>
  <line x1="400" y1="252" x2="660" y2="252" class="cls-sep"/>
  <text x="410" y="268" class="cls-attr">- buildingItems_: QMap&lt;int,BuildingItem*&gt;</text>
  <text x="410" y="284" class="cls-attr">- roadItems_: QList&lt;RoadItem*&gt;</text>
  <text x="410" y="300" class="cls-attr">- startPointId_: int</text>
  <text x="410" y="316" class="cls-attr">- endPointId_: int</text>
  <line x1="400" y1="324" x2="660" y2="324" class="cls-sep"/>
  <text x="410" y="340" class="cls-method">+ loadFromGraph(graph: Graph)</text>
  <text x="410" y="356" class="cls-method">+ highlightPath(path: vector&lt;int&gt;)</text>
  <text x="410" y="372" class="cls-method">+ clearPathHighlight()</text>
  <text x="410" y="388" class="cls-method">+ setStartPoint(id: int)</text>
  <text x="410" y="404" class="cls-method">+ setEndPoint(id: int)</text>
  <text x="410" y="420" class="cls-method">+ buildingClicked(id: int) «signal»</text>

  <!-- ============ BuildingItem ============ -->
  <rect x="700" y="200" width="240" height="220" class="cls-box" rx="4"/>
  <rect x="700" y="200" width="240" height="28" fill="#2E7D32" rx="4"/>
  <rect x="700" y="220" width="240" height="8" fill="#2E7D32"/>
  <text x="820" y="219" class="cls-name" fill="white">BuildingItem</text>
  <line x1="700" y1="228" x2="940" y2="228" class="cls-sep"/>
  <text x="710" y="244" class="cls-attr">- building_: const Building*</text>
  <text x="710" y="260" class="cls-attr">- selected_: bool</text>
  <text x="710" y="276" class="cls-attr">- hovered_: bool</text>
  <text x="710" y="292" class="cls-attr">- labelAbove_: bool</text>
  <text x="710" y="308" class="cls-attr">- labelScale_: qreal</text>
  <line x1="700" y1="316" x2="940" y2="316" class="cls-sep"/>
  <text x="710" y="332" class="cls-method">+ paint(painter, option, widget)</text>
  <text x="710" y="348" class="cls-method">+ setSelected(selected: bool)</text>
  <text x="710" y="364" class="cls-method">+ setLabelScale(scale: qreal)</text>
  <text x="710" y="380" class="cls-method">+ clicked(id: int) «signal»</text>
  <text x="710" y="396" class="cls-method">- typeColor(): QColor</text>

  <!-- ============ CharacterItem ============ -->
  <rect x="700" y="460" width="240" height="200" class="cls-box" rx="4"/>
  <rect x="700" y="460" width="240" height="28" fill="#2E7D32" rx="4"/>
  <rect x="700" y="480" width="240" height="8" fill="#2E7D32"/>
  <text x="820" y="479" class="cls-name" fill="white">CharacterItem</text>
  <line x1="700" y1="488" x2="940" y2="488" class="cls-sep"/>
  <text x="710" y="504" class="cls-attr">- direction_: Direction</text>
  <text x="710" y="520" class="cls-attr">- speed_: double</text>
  <text x="710" y="536" class="cls-attr">- walking_: bool</text>
  <text x="710" y="552" class="cls-attr">- walkFrame_: int</text>
  <line x1="700" y1="560" x2="940" y2="560" class="cls-sep"/>
  <text x="710" y="576" class="cls-method">+ paint(painter, option, widget)</text>
  <text x="710" y="592" class="cls-method">+ setDirection(d: Direction)</text>
  <text x="710" y="608" class="cls-method">+ setWalking(w: bool)</text>
  <text x="710" y="624" class="cls-method">+ advanceFrame()</text>
  <text x="710" y="640" class="cls-method">- drawHead/Body/Legs/Arms()</text>

  <!-- ============ NpcItem ============ -->
  <rect x="980" y="460" width="260" height="220" class="cls-box" rx="4"/>
  <rect x="980" y="460" width="260" height="28" fill="#2E7D32" rx="4"/>
  <rect x="980" y="480" width="260" height="8" fill="#2E7D32"/>
  <text x="1110" y="479" class="cls-name" fill="white">NpcItem</text>
  <line x1="980" y1="488" x2="1240" y2="488" class="cls-sep"/>
  <text x="990" y="504" class="cls-attr">- name_: QString</text>
  <text x="990" y="520" class="cls-attr">- dialog_: QString</text>
  <text x="990" y="536" class="cls-attr">- outfit_: Outfit</text>
  <text x="990" y="552" class="cls-attr">- graph_: const Graph*</text>
  <text x="990" y="568" class="cls-attr">- targetBuildingId_: int</text>
  <text x="990" y="584" class="cls-attr">- moveState_: MoveState</text>
  <line x1="980" y1="592" x2="1240" y2="592" class="cls-sep"/>
  <text x="990" y="608" class="cls-method">+ updateMovement()</text>
  <text x="990" y="624" class="cls-method">+ setGraph(graph: const Graph*)</text>
  <text x="990" y="640" class="cls-method">+ clicked(name, dialog) «signal»</text>
  <text x="990" y="656" class="cls-method">- pickNewTarget()</text>

  <!-- ============ WeatherOverlay ============ -->
  <rect x="980" y="200" width="200" height="160" class="cls-box" rx="4"/>
  <rect x="980" y="200" width="200" height="28" fill="#2E7D32" rx="4"/>
  <rect x="980" y="220" width="200" height="8" fill="#2E7D32"/>
  <text x="1080" y="219" class="cls-name" fill="white">WeatherOverlay</text>
  <line x1="980" y1="228" x2="1180" y2="228" class="cls-sep"/>
  <text x="990" y="244" class="cls-attr">- weather_: WeatherType</text>
  <text x="990" y="260" class="cls-attr">- particles_: QVector&lt;Particle&gt;</text>
  <text x="990" y="276" class="cls-attr">- areaW_/areaH_: double</text>
  <line x1="980" y1="284" x2="1180" y2="284" class="cls-sep"/>
  <text x="990" y="300" class="cls-method">+ setWeather(type)</text>
  <text x="990" y="316" class="cls-method">+ setAreaSize(w, h)</text>
  <text x="990" y="332" class="cls-method">+ advance(phase)</text>
  <text x="990" y="348" class="cls-method">- initParticles()</text>

  <!-- ============ AdminDialog ============ -->
  <rect x="1210" y="200" width="250" height="200" class="cls-box" rx="4"/>
  <rect x="1210" y="200" width="250" height="28" fill="#2E7D32" rx="4"/>
  <rect x="1210" y="220" width="250" height="8" fill="#2E7D32"/>
  <text x="1335" y="219" class="cls-name" fill="white">AdminDialog</text>
  <line x1="1210" y1="228" x2="1460" y2="228" class="cls-sep"/>
  <text x="1220" y="244" class="cls-attr">- tabWidget_: QTabWidget*</text>
  <text x="1220" y="260" class="cls-attr">- listWidget_: QListWidget*</text>
  <text x="1220" y="276" class="cls-attr">- nameEdit_: QLineEdit*</text>
  <text x="1220" y="292" class="cls-attr">- roadFromCombo_: QComboBox*</text>
  <line x1="1210" y1="300" x2="1460" y2="300" class="cls-sep"/>
  <text x="1220" y="316" class="cls-method">+ onAdd() / onUpdate()</text>
  <text x="1220" y="332" class="cls-method">+ onDelete()</text>
  <text x="1220" y="348" class="cls-method">+ onAddRoad() / onDeleteRoad()</text>
  <text x="1220" y="364" class="cls-method">+ onResetDefaults()</text>

  <!-- ============ RoadItem ============ -->
  <rect x="400" y="480" width="220" height="140" class="cls-box" rx="4"/>
  <rect x="400" y="480" width="220" height="28" fill="#2E7D32" rx="4"/>
  <rect x="400" y="500" width="220" height="8" fill="#2E7D32"/>
  <text x="510" y="499" class="cls-name" fill="white">RoadItem</text>
  <line x1="400" y1="508" x2="620" y2="508" class="cls-sep"/>
  <text x="410" y="524" class="cls-attr">- from_: QPointF</text>
  <text x="410" y="540" class="cls-attr">- to_: QPointF</text>
  <text x="410" y="556" class="cls-attr">- highlighted_: bool</text>
  <line x1="400" y1="564" x2="620" y2="564" class="cls-sep"/>
  <text x="410" y="580" class="cls-method">+ paint(...)</text>
  <text x="410" y="596" class="cls-method">+ setHighlighted(h: bool)</text>

  <!-- ============ 数据层 ============ -->
  <!-- DatabaseManager -->
  <rect x="700" y="720" width="280" height="280" class="cls-box" rx="4"/>
  <rect x="700" y="720" width="280" height="28" fill="#E65100" rx="4"/>
  <rect x="700" y="740" width="280" height="8" fill="#E65100"/>
  <text x="840" y="739" class="cls-name" fill="white">DatabaseManager «singleton»</text>
  <line x1="700" y1="748" x2="980" y2="748" class="cls-sep"/>
  <text x="710" y="764" class="cls-attr">- db_: QSqlDatabase</text>
  <text x="710" y="780" class="cls-attr">- initialized_: bool</text>
  <line x1="700" y1="788" x2="980" y2="788" class="cls-sep"/>
  <text x="710" y="804" class="cls-method">+ instance(): DatabaseManager&amp;</text>
  <text x="710" y="820" class="cls-method">+ init(dbPath: QString): bool</text>
  <text x="710" y="836" class="cls-method">+ loadAllBuildings(): vector&lt;Building&gt;</text>
  <text x="710" y="852" class="cls-method">+ addBuilding(b: Building): bool</text>
  <text x="710" y="868" class="cls-method">+ updateBuilding(b: Building): bool</text>
  <text x="710" y="884" class="cls-method">+ deleteBuilding(id: int): bool</text>
  <text x="710" y="900" class="cls-method">+ loadAllRoads(): vector&lt;Road&gt;</text>
  <text x="710" y="916" class="cls-method">+ addRoad(from, to, w): bool</text>
  <text x="710" y="932" class="cls-method">+ deleteRoad(from, to): bool</text>
  <text x="710" y="948" class="cls-method">+ resetToDefaultData(): bool</text>
  <text x="710" y="964" class="cls-method">+ importPlaceIntroductions(dir)</text>

  <!-- CampusData -->
  <rect x="1020" y="720" width="240" height="120" class="cls-box" rx="4"/>
  <rect x="1020" y="720" width="240" height="28" fill="#E65100" rx="4"/>
  <rect x="1020" y="740" width="240" height="8" fill="#E65100"/>
  <text x="1140" y="739" class="cls-name" fill="white">CampusData «namespace»</text>
  <line x1="1020" y1="748" x2="1260" y2="748" class="cls-sep"/>
  <text x="1030" y="768" class="cls-attr">(无成员变量)</text>
  <line x1="1020" y1="778" x2="1260" y2="778" class="cls-sep"/>
  <text x="1030" y="794" class="cls-method">+ populate(graph: Graph&amp;)</text>
  <text x="1030" y="810" class="cls-attr" fill="#78909C">// 45栋建筑 + 40条道路</text>
  <text x="1030" y="826" class="cls-attr" fill="#78909C">// 种子数据，首次启动加载</text>

  <!-- ============ 模型层 ============ -->
  <!-- Building -->
  <rect x="1310" y="720" width="240" height="300" class="cls-box" rx="4"/>
  <rect x="1310" y="720" width="240" height="28" fill="#6A1B9A" rx="4"/>
  <rect x="1310" y="740" width="240" height="8" fill="#6A1B9A"/>
  <text x="1430" y="739" class="cls-name" fill="white">Building</text>
  <line x1="1310" y1="748" x2="1550" y2="748" class="cls-sep"/>
  <text x="1320" y="764" class="cls-attr">- id_: int</text>
  <text x="1320" y="780" class="cls-attr">- name_: QString</text>
  <text x="1320" y="796" class="cls-attr">- x_: double</text>
  <text x="1320" y="812" class="cls-attr">- y_: double</text>
  <text x="1320" y="828" class="cls-attr">- type_: BuildingType</text>
  <text x="1320" y="844" class="cls-attr">- info_: QString</text>
  <text x="1320" y="860" class="cls-attr">- openHours_: QString</text>
  <text x="1320" y="876" class="cls-attr">- floors_: int</text>
  <text x="1320" y="892" class="cls-attr">- nodeKind_: int</text>
  <line x1="1310" y1="900" x2="1550" y2="900" class="cls-sep"/>
  <text x="1320" y="916" class="cls-method">+ id(): int</text>
  <text x="1320" y="932" class="cls-method">+ name(): const QString&amp;</text>
  <text x="1320" y="948" class="cls-method">+ x(): double / y(): double</text>
  <text x="1320" y="964" class="cls-method">+ type(): BuildingType</text>
  <text x="1320" y="980" class="cls-method">+ setInfo(info: QString)</text>
  <text x="1320" y="996" class="cls-method">+ setPosition(x, y)</text>
  <text x="1320" y="1012" class="cls-method">+ description(): QString</text>

  <!-- Edge -->
  <rect x="1580" y="720" width="200" height="180" class="cls-box" rx="4"/>
  <rect x="1580" y="720" width="200" height="28" fill="#6A1B9A" rx="4"/>
  <rect x="1580" y="740" width="200" height="8" fill="#6A1B9A"/>
  <text x="1680" y="739" class="cls-name" fill="white">Edge «struct»</text>
  <line x1="1580" y1="748" x2="1780" y2="748" class="cls-sep"/>
  <text x="1590" y="764" class="cls-attr">+ to: int</text>
  <text x="1590" y="780" class="cls-attr">+ weight: double</text>
  <text x="1590" y="796" class="cls-attr">+ shelter: bool</text>
  <text x="1590" y="812" class="cls-attr">+ tree: int</text>
  <text x="1590" y="828" class="cls-attr">+ lake: bool</text>
  <line x1="1580" y1="836" x2="1780" y2="836" class="cls-sep"/>
  <text x="1590" y="852" class="cls-method">+ Edge(to, weight)</text>
  <text x="1590" y="868" class="cls-method">+ Edge(to, w, shelter,</text>
  <text x="1590" y="884" class="cls-method">       tree, lake)</text>

  <!-- Graph -->
  <rect x="1310" y="1060" width="470" height="300" class="cls-box" rx="4"/>
  <rect x="1310" y="1060" width="470" height="28" fill="#6A1B9A" rx="4"/>
  <rect x="1310" y="1080" width="470" height="8" fill="#6A1B9A"/>
  <text x="1545" y="1079" class="cls-name" fill="white">Graph</text>
  <line x1="1310" y1="1088" x2="1780" y2="1088" class="cls-sep"/>
  <text x="1320" y="1104" class="cls-attr">- buildings_: unordered_map&lt;int, Building&gt;</text>
  <text x="1320" y="1120" class="cls-attr">- adjacency_: unordered_map&lt;int, vector&lt;Edge&gt;&gt;</text>
  <text x="1320" y="1136" class="cls-attr">- roadCount_: int</text>
  <line x1="1310" y1="1144" x2="1780" y2="1144" class="cls-sep"/>
  <text x="1320" y="1160" class="cls-method">+ addBuilding(b: Building)</text>
  <text x="1320" y="1176" class="cls-method">+ getBuilding(id: int): const Building*</text>
  <text x="1320" y="1192" class="cls-method">+ addRoad(from, to, distance)</text>
  <text x="1320" y="1208" class="cls-method">+ addRoad(from, to, dist, shelter, tree, lake)</text>
  <text x="1320" y="1224" class="cls-method">+ neighbors(id: int): const vector&lt;Edge&gt;&amp;</text>
  <text x="1320" y="1240" class="cls-method">+ updatePosition(id, x, y)</text>
  <text x="1320" y="1256" class="cls-method">+ dijkstra(start, end): vector&lt;int&gt;</text>
  <text x="1320" y="1272" class="cls-method">+ dijkstraWithWeather(start, end, mode)</text>
  <text x="1320" y="1288" class="cls-method">+ buildingCount(): int</text>
  <text x="1320" y="1304" class="cls-method">+ roadCount(): int</text>

  <!-- ============ 关系线 ============ -->
  <!-- MainWindow → QMainWindow (继承) -->
  <line x1="200" y1="200" x2="150" y2="135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- MapScene → QGraphicsScene (继承) -->
  <line x1="530" y1="200" x2="390" y2="135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- BuildingItem → QGraphicsObject (继承) -->
  <line x1="820" y1="200" x2="630" y2="135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- CharacterItem → QGraphicsObject (继承) -->
  <path d="M 820 460 L 820 420 L 700 420 L 700 200 L 630 200 L 630 135" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- NpcItem → QGraphicsObject (继承) -->
  <path d="M 1110 460 L 1110 430 L 700 430 L 700 200 L 630 200 L 630 135" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- AdminDialog → QDialog (继承) -->
  <line x1="1335" y1="200" x2="860" y2="135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <!-- WeatherOverlay → QGraphicsItem (继承) -->
  <line x1="1080" y1="200" x2="1070" y2="135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>

  <!-- MainWindow → MapScene (组合) -->
  <line x1="360" y1="280" x2="400" y2="280" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="380" y="275" class="rel-label">1</text>
  <!-- MainWindow → Graph (组合) -->
  <path d="M 360 340 L 380 340 L 380 1100 L 1310 1100" fill="none" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="370" y="335" class="rel-label">1</text>
  <!-- MainWindow → CharacterItem (组合) -->
  <path d="M 360 310 L 680 310 L 680 460 L 700 460" fill="none" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <!-- MainWindow → NpcItem (组合) -->
  <path d="M 360 370 L 660 370 L 660 680 L 980 680 L 980 680" fill="none" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <!-- MainWindow → AdminDialog (关联) -->
  <path d="M 320 520 L 320 690 L 1335 690 L 1335 400" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)" stroke-dasharray="0"/>

  <!-- MapScene → BuildingItem (组合) -->
  <line x1="660" y1="260" x2="700" y2="260" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="670" y="255" class="rel-label">0..*</text>
  <!-- MapScene → RoadItem (组合) -->
  <line x1="510" y1="440" x2="510" y2="480" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="515" y="465" class="rel-label">0..*</text>
  <!-- MapScene → Graph (关联) -->
  <path d="M 660 400 L 680 400 L 680 1060 L 1310 1060" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)"/>

  <!-- Graph → Building (组合) -->
  <line x1="1430" y1="1060" x2="1430" y2="1020" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="1435" y="1045" class="rel-label">0..*</text>
  <!-- Graph → Edge (组合) -->
  <line x1="1680" y1="1060" x2="1680" y2="900" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="1685" y="980" class="rel-label">0..*</text>

  <!-- BuildingItem → Building (关联) -->
  <path d="M 940 240 L 1180 240 L 1180 600 L 1430 600 L 1430 720" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)"/>
  <text x="1200" y="235" class="rel-label">building</text>

  <!-- NpcItem → Graph (关联) -->
  <path d="M 1240 560 L 1280 560 L 1280 1130 L 1310 1130" fill="none" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)"/>
  <text x="1285" y="555" class="rel-label">graph</text>

  <!-- DatabaseManager → Building (关联) -->
  <line x1="980" y1="800" x2="1310" y2="800" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)"/>
  <text x="1100" y="795" class="rel-label">CRUD</text>

  <!-- CampusData → Graph (依赖) -->
  <path d="M 1140 840 L 1140 1060 L 1310 1060" fill="none" stroke="#78909C" stroke-width="1.5" stroke-dasharray="5,3" marker-end="url(#depend)"/>
  <text x="1145" y="950" class="rel-label">«uses»</text>

  <!-- DatabaseManager → MainWindow (被使用) -->
  <path d="M 840 720 L 840 660 L 200 660 L 200 520" fill="none" stroke="#78909C" stroke-width="1.5" stroke-dasharray="5,3" marker-end="url(#depend)"/>

  <!-- ============ 图例 ============ -->
  <rect x="40" y="1080" width="400" height="200" class="legend-box" rx="6"/>
  <text x="60" y="1105" font-size="13" font-weight="bold" fill="#37474F">图例 (UML 关系符号)</text>
  <!-- 继承 -->
  <line x1="60" y1="1135" x2="120" y2="1135" stroke="#37474F" stroke-width="1.5" marker-end="url(#inherit)"/>
  <text x="140" y="1139" font-size="11" fill="#37474F">继承 (Inheritance) — 子类指向父类</text>
  <!-- 组合 -->
  <line x1="60" y1="1165" x2="120" y2="1165" stroke="#37474F" stroke-width="1.5" marker-start="url(#compose)"/>
  <text x="140" y="1169" font-size="11" fill="#37474F">组合 (Composition) — 整体创建/销毁部分</text>
  <!-- 聚合 -->
  <line x1="60" y1="1195" x2="120" y2="1195" stroke="#37474F" stroke-width="1.5" marker-start="url(#aggregate)"/>
  <text x="140" y="1199" font-size="11" fill="#37474F">聚合 (Aggregation) — 整体引用部分</text>
  <!-- 关联 -->
  <line x1="60" y1="1225" x2="120" y2="1225" stroke="#37474F" stroke-width="1.5" marker-end="url(#assoc)"/>
  <text x="140" y="1229" font-size="11" fill="#37474F">关联 (Association) — 持有引用</text>
  <!-- 依赖 -->
  <line x1="60" y1="1255" x2="120" y2="1255" stroke="#78909C" stroke-width="1.5" stroke-dasharray="5,3" marker-end="url(#depend)"/>
  <text x="140" y="1259" font-size="11" fill="#37474F">依赖 (Dependency) — 临时使用</text>

  <!-- 颜色说明 -->
  <rect x="470" y="1080" width="420" height="200" class="legend-box" rx="6"/>
  <text x="490" y="1105" font-size="13" font-weight="bold" fill="#37474F">分层颜色说明</text>
  <rect x="490" y="1120" width="20" height="20" fill="#1565C0" rx="3"/>
  <text x="520" y="1135" font-size="11" fill="#37474F">控制器 (MainWindow) — 负责UI布局与事件调度</text>
  <rect x="490" y="1150" width="20" height="20" fill="#2E7D32" rx="3"/>
  <text x="520" y="1165" font-size="11" fill="#37474F">视图层 (View) — 图元绘制与交互</text>
  <rect x="490" y="1180" width="20" height="20" fill="#E65100" rx="3"/>
  <text x="520" y="1195" font-size="11" fill="#37474F">数据层 (Data) — SQLite持久化与种子数据</text>
  <rect x="490" y="1210" width="20" height="20" fill="#6A1B9A" rx="3"/>
  <text x="520" y="1225" font-size="11" fill="#37474F">模型层 (Model) — 纯数据结构与算法</text>
  <rect x="490" y="1240" width="20" height="20" fill="#E3F2FD" stroke="#1565C0" rx="3"/>
  <text x="520" y="1255" font-size="11" fill="#37474F">Qt 基类 — 框架提供的父类</text>
</svg>
'''

# ============================================================
#  用例图
# ============================================================
USE_CASE_DIAGRAM = r'''
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 800" font-family="Microsoft YaHei,SimHei,sans-serif">
  <defs>
    <style>
      .actor { fill: #1565C0; }
      .uc { fill: #E3F2FD; stroke: #1565C0; stroke-width: 1.5; }
      .uc-text { font-size: 12px; text-anchor: middle; fill: #263238; }
      .sys-border { fill: none; stroke: #90A4AE; stroke-width: 2; stroke-dasharray: 8,4; }
      .title { font-size: 18px; font-weight: bold; text-anchor: middle; fill: #263238; }
    </style>
  </defs>
  <rect width="1200" height="800" fill="#FAFAFA"/>
  <text x="600" y="30" class="title">CampusNavigator 用例图</text>

  <!-- 系统边界 -->
  <rect x="280" y="60" width="640" height="700" class="sys-border" rx="10"/>
  <text x="600" y="50" font-size="13" font-weight="bold" fill="#546E7A" text-anchor="middle">校园探索与智能导航模拟系统</text>

  <!-- 演员：普通用户 -->
  <g transform="translate(120, 300)">
    <circle cx="0" cy="0" r="18" class="actor"/>
    <line x1="0" y1="18" x2="0" y2="55" stroke="#1565C0" stroke-width="3"/>
    <line x1="-20" y1="35" x2="20" y2="35" stroke="#1565C0" stroke-width="3"/>
    <line x1="0" y1="55" x2="-15" y2="80" stroke="#1565C0" stroke-width="3"/>
    <line x1="0" y1="55" x2="15" y2="80" stroke="#1565C0" stroke-width="3"/>
    <text x="0" y="105" font-size="14" font-weight="bold" text-anchor="middle" fill="#1565C0">普通用户</text>
    <text x="0" y="122" font-size="11" text-anchor="middle" fill="#78909C">(新生/访客)</text>
  </g>

  <!-- 演员：管理员 -->
  <g transform="translate(1080, 400)">
    <circle cx="0" cy="0" r="18" fill="#E65100"/>
    <line x1="0" y1="18" x2="0" y2="55" stroke="#E65100" stroke-width="3"/>
    <line x1="-20" y1="35" x2="20" y2="35" stroke="#E65100" stroke-width="3"/>
    <line x1="0" y1="55" x2="-15" y2="80" stroke="#E65100" stroke-width="3"/>
    <line x1="0" y1="55" x2="15" y2="80" stroke="#E65100" stroke-width="3"/>
    <text x="0" y="105" font-size="14" font-weight="bold" text-anchor="middle" fill="#E65100">管理员</text>
  </g>

  <!-- 用例 -->
  <ellipse cx="600" cy="100" rx="110" ry="28" class="uc"/><text x="600" y="105" class="uc-text">浏览校园地图</text>
  <ellipse cx="600" cy="165" rx="110" ry="28" class="uc"/><text x="600" y="170" class="uc-text">搜索建筑</text>
  <ellipse cx="600" cy="230" rx="110" ry="28" class="uc"/><text x="600" y="235" class="uc-text">查看建筑信息</text>
  <ellipse cx="600" cy="295" rx="110" ry="28" class="uc"/><text x="600" y="300" class="uc-text">规划导航路径</text>
  <ellipse cx="600" cy="360" rx="110" ry="28" class="uc"/><text x="600" y="365" class="uc-text">WASD角色漫游</text>
  <ellipse cx="600" cy="425" rx="110" ry="28" class="uc"/><text x="600" y="430" class="uc-text">自动路径导航</text>
  <ellipse cx="600" cy="490" rx="110" ry="28" class="uc"/><text x="600" y="495" class="uc-text">切换昼夜/天气</text>
  <ellipse cx="600" cy="555" rx="110" ry="28" class="uc"/><text x="600" y="560" class="uc-text">NPC对话交互</text>
  <ellipse cx="600" cy="620" rx="110" ry="28" class="uc"/><text x="600" y="625" class="uc-text">新生引导模式</text>
  <ellipse cx="600" cy="685" rx="110" ry="28" class="uc"/><text x="600" y="690" class="uc-text">访客导览模式</text>
  <ellipse cx="600" cy="735" rx="110" ry="28" class="uc" stroke="#E65100"/><text x="600" y="740" class="uc-text" fill="#E65100">管理建筑/道路(CRUD)</text>

  <!-- 连线：普通用户 -->
  <line x1="140" y1="300" x2="490" y2="100" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="165" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="230" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="295" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="360" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="425" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="490" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="555" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="620" stroke="#90A4AE" stroke-width="1"/>
  <line x1="140" y1="300" x2="490" y2="685" stroke="#90A4AE" stroke-width="1"/>

  <!-- 连线：管理员 -->
  <line x1="1060" y1="400" x2="710" y2="735" stroke="#90A4AE" stroke-width="1"/>
  <line x1="1060" y1="400" x2="710" y2="100" stroke="#90A4AE" stroke-width="1" stroke-dasharray="4,2"/>
  <text x="850" y="120" font-size="10" fill="#78909C">«include»</text>
</svg>
'''

# ============================================================
#  系统架构图
# ============================================================
ARCH_DIAGRAM = r'''
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 700" font-family="Microsoft YaHei,SimHei,sans-serif">
  <defs>
    <style>
      .layer-bg { fill: #FFFFFF; stroke: #BDBDBD; stroke-width: 1; rx: 8; }
      .layer-title { font-size: 16px; font-weight: bold; text-anchor: middle; }
      .module { fill: #E3F2FD; stroke: #1565C0; stroke-width: 1.5; rx: 6; }
      .module-text { font-size: 12px; text-anchor: middle; fill: #263238; }
      .arrow { stroke: #546E7A; stroke-width: 2; fill: none; }
      .title { font-size: 20px; font-weight: bold; text-anchor: middle; fill: #263238; }
    </style>
    <marker id="arr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto">
      <path d="M0,0 L9,5 L0,10 Z" fill="#546E7A"/>
    </marker>
  </defs>
  <rect width="1200" height="700" fill="#FAFAFA"/>
  <text x="600" y="35" class="title">CampusNavigator 三层架构图</text>

  <!-- 用户层 -->
  <rect x="50" y="60" width="1100" height="80" fill="#FFF3E0" stroke="#FF9800" stroke-width="2" rx="8"/>
  <text x="600" y="85" class="layer-title" fill="#E65100">用户交互层</text>
  <rect x="150" y="95" width="140" height="35" class="module" rx="6"/><text x="220" y="117" class="module-text">键盘/鼠标输入</text>
  <rect x="350" y="95" width="140" height="35" class="module" rx="6"/><text x="420" y="117" class="module-text">按钮/搜索框</text>
  <rect x="550" y="95" width="140" height="35" class="module" rx="6"/><text x="620" y="117" class="module-text">滚轮缩放/拖拽</text>
  <rect x="750" y="95" width="140" height="35" class="module" rx="6"/><text x="820" y="117" class="module-text">模式切换</text>
  <rect x="950" y="95" width="140" height="35" class="module" rx="6"/><text x="1020" y="117" class="module-text">NPC点击对话</text>

  <!-- 箭头 -->
  <line x1="600" y1="140" x2="600" y2="170" class="arrow" marker-end="url(#arr)"/>

  <!-- 视图层 -->
  <rect x="50" y="170" width="1100" height="200" fill="#E8F5E9" stroke="#4CAF50" stroke-width="2" rx="8"/>
  <text x="600" y="195" class="layer-title" fill="#2E7D32">视图层 (View) — Qt Graphics View Framework</text>
  <rect x="80" y="210" width="160" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="160" y="232" class="module-text" font-weight="bold">MainWindow</text>
  <text x="160" y="250" class="module-text" font-size="10" fill="#558B2F">主窗口·UI布局·事件调度</text>
  <rect x="270" y="210" width="140" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="340" y="232" class="module-text" font-weight="bold">MapScene</text>
  <text x="340" y="250" class="module-text" font-size="10" fill="#558B2F">场景·图元管理·路径高亮</text>
  <rect x="440" y="210" width="130" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="505" y="232" class="module-text" font-weight="bold">BuildingItem</text>
  <text x="505" y="250" class="module-text" font-size="10" fill="#558B2F">建筑标签·交互</text>
  <rect x="595" y="210" width="120" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="655" y="232" class="module-text" font-weight="bold">CharacterItem</text>
  <text x="655" y="250" class="module-text" font-size="10" fill="#558B2F">角色·4方向动画</text>
  <rect x="740" y="210" width="120" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="800" y="232" class="module-text" font-weight="bold">NpcItem</text>
  <text x="800" y="250" class="module-text" font-size="10" fill="#558B2F">NPC·自主移动·对话</text>
  <rect x="885" y="210" width="120" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="945" y="232" class="module-text" font-weight="bold">WeatherOverlay</text>
  <text x="945" y="250" class="module-text" font-size="10" fill="#558B2F">天气粒子系统</text>
  <rect x="1030" y="210" width="100" height="60" fill="#C8E6C9" stroke="#2E7D32" stroke-width="1.5" rx="6"/>
  <text x="1080" y="232" class="module-text" font-weight="bold">AdminDialog</text>
  <text x="1080" y="250" class="module-text" font-size="10" fill="#558B2F">CRUD管理</text>

  <!-- 4定时器 -->
  <rect x="200" y="300" width="800" height="50" fill="#FFF9C4" stroke="#FBC02D" stroke-width="1.5" rx="6"/>
  <text x="600" y="322" font-size="13" font-weight="bold" text-anchor="middle" fill="#F57F17">4 定时器协作系统 (30 FPS 游戏循环)</text>
  <text x="600" y="340" font-size="11" text-anchor="middle" fill="#795548">moveTimer(角色移动) · navTimer(自动导航) · npcTimer(NPC移动) · animTimer(动画帧推进)</text>

  <!-- 箭头 -->
  <line x1="600" y1="370" x2="600" y2="400" class="arrow" marker-end="url(#arr)"/>

  <!-- 数据层 -->
  <rect x="50" y="400" width="550" height="130" fill="#FFF3E0" stroke="#FF9800" stroke-width="2" rx="8"/>
  <text x="325" y="425" class="layer-title" fill="#E65100">数据层 (Data)</text>
  <rect x="80" y="440" width="220" height="75" fill="#FFE0B2" stroke="#E65100" stroke-width="1.5" rx="6"/>
  <text x="190" y="462" class="module-text" font-weight="bold">DatabaseManager «singleton»</text>
  <text x="190" y="480" class="module-text" font-size="10" fill="#795548">SQLite CRUD · 建表·迁移</text>
  <text x="190" y="496" class="module-text" font-size="10" fill="#795548">预处理语句防注入</text>
  <rect x="330" y="440" width="220" height="75" fill="#FFE0B2" stroke="#E65100" stroke-width="1.5" rx="6"/>
  <text x="440" y="462" class="module-text" font-weight="bold">CampusData «namespace»</text>
  <text x="440" y="480" class="module-text" font-size="10" fill="#795548">45栋建筑·40条道路</text>
  <text x="440" y="496" class="module-text" font-size="10" fill="#795548">种子数据(首次启动)</text>

  <!-- 模型层 -->
  <rect x="620" y="400" width="530" height="130" fill="#F3E5F5" stroke="#9C27B0" stroke-width="2" rx="8"/>
  <text x="885" y="425" class="layer-title" fill="#6A1B9A">模型层 (Model) — 纯数据结构与算法</text>
  <rect x="650" y="440" width="140" height="75" fill="#E1BEE7" stroke="#6A1B9A" stroke-width="1.5" rx="6"/>
  <text x="720" y="462" class="module-text" font-weight="bold">Building</text>
  <text x="720" y="480" class="module-text" font-size="10" fill="#6A1B9A">id·name·坐标·类型</text>
  <text x="720" y="496" class="module-text" font-size="10" fill="#6A1B9A">11种建筑类型枚举</text>
  <rect x="810" y="440" width="140" height="75" fill="#E1BEE7" stroke="#6A1B9A" stroke-width="1.5" rx="6"/>
  <text x="880" y="462" class="module-text" font-weight="bold">Edge «struct»</text>
  <text x="880" y="480" class="module-text" font-size="10" fill="#6A1B9A">to·weight(距离)</text>
  <text x="880" y="496" class="module-text" font-size="10" fill="#6A1B9A">天气标签(shelter/tree/lake)</text>
  <rect x="970" y="440" width="160" height="75" fill="#E1BEE7" stroke="#6A1B9A" stroke-width="1.5" rx="6"/>
  <text x="1050" y="462" class="module-text" font-weight="bold">Graph</text>
  <text x="1050" y="480" class="module-text" font-size="10" fill="#6A1B9A">邻接表·Dijkstra最短路径</text>
  <text x="1050" y="496" class="module-text" font-size="10" fill="#6A1B9A">天气感知路径规划</text>

  <!-- 底层 -->
  <line x1="325" y1="530" x2="325" y2="560" class="arrow" marker-end="url(#arr)"/>
  <line x1="885" y1="530" x2="885" y2="560" class="arrow" marker-end="url(#arr)"/>
  <rect x="150" y="560" width="350" height="60" fill="#ECEFF1" stroke="#607D8B" stroke-width="2" rx="8"/>
  <text x="325" y="585" font-size="14" font-weight="bold" text-anchor="middle" fill="#37474F">SQLite 3 数据库</text>
  <text x="325" y="605" font-size="11" text-anchor="middle" fill="#607D8B">campus.db · buildings表 + roads表</text>
  <rect x="700" y="560" width="350" height="60" fill="#ECEFF1" stroke="#607D8B" stroke-width="2" rx="8"/>
  <text x="875" y="585" font-size="14" font-weight="bold" text-anchor="middle" fill="#37474F">STL 容器</text>
  <text x="875" y="605" font-size="11" text-anchor="middle" fill="#607D8B">unordered_map · vector · priority_queue</text>

  <!-- 数据流标注 -->
  <text x="600" y="545" font-size="11" fill="#78909C" text-anchor="middle">数据持久化 ←→ 内存数据结构</text>
</svg>
'''

# ============================================================
#  Dijkstra 算法执行过程图
# ============================================================
DIJKSTRA_VIZ = r'''
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 800" font-family="Microsoft YaHei,SimHei,sans-serif">
  <defs>
    <style>
      .node { stroke: #37474F; stroke-width: 2; }
      .node-start { fill: #4CAF50; }
      .node-visited { fill: #FFD54F; }
      .node-current { fill: #FF7043; stroke: #D32F2F; stroke-width: 3; }
      .node-end { fill: #F44336; }
      .node-unvisited { fill: #FFFFFF; }
      .edge { stroke: #BDBDBD; stroke-width: 2; }
      .edge-relaxed { stroke: #FF7043; stroke-width: 3; }
      .edge-path { stroke: #4CAF50; stroke-width: 4; }
      .label { font-size: 12px; text-anchor: middle; fill: #263238; }
      .dist { font-size: 11px; text-anchor: middle; fill: #1565C0; font-weight: bold; }
      .title { font-size: 18px; font-weight: bold; text-anchor: middle; fill: #263238; }
      .step { font-size: 14px; fill: #37474F; }
    </style>
  </defs>
  <rect width="1200" height="800" fill="#FAFAFA"/>
  <text x="600" y="30" class="title">Dijkstra 最短路径算法执行过程示例</text>
  <text x="600" y="52" font-size="13" text-anchor="middle" fill="#546E7A">从 北门(0) 到 宿舍16-20(42) 的最短路径计算</text>

  <!-- 步骤1：初始化 -->
  <text x="80" y="90" class="step" font-weight="bold">步骤1：初始化</text>
  <text x="80" y="110" class="step" font-size="12">dist[0]=0, 其余=∞, 优先队列入(0, 起点)</text>

  <!-- 图节点位置 -->
  <!-- 北门 -->
  <circle cx="150" cy="180" r="22" class="node node-start"/>
  <text x="150" y="185" class="label" fill="white" font-weight="bold">0</text>
  <text x="150" y="155" class="label" font-size="10">北门</text>
  <text x="150" y="215" class="dist">dist=0</text>

  <!-- 教学楼1 -->
  <circle cx="350" cy="180" r="22" class="node node-visited"/>
  <text x="350" y="185" class="label" fill="#263238" font-weight="bold">7</text>
  <text x="350" y="155" class="label" font-size="10">教学楼1</text>
  <text x="350" y="215" class="dist">dist=80</text>

  <!-- 材料学院 -->
  <circle cx="550" cy="180" r="22" class="node node-visited"/>
  <text x="550" y="185" class="label" fill="#263238" font-weight="bold">11</text>
  <text x="550" y="155" class="label" font-size="10">材料学院</text>
  <text x="550" y="215" class="dist">dist=200</text>

  <!-- 环安学院 -->
  <circle cx="750" cy="180" r="22" class="node node-visited"/>
  <text x="750" y="185" class="label" fill="#263238" font-weight="bold">17</text>
  <text x="750" y="155" class="label" font-size="10">环安学院</text>
  <text x="750" y="215" class="dist">dist=330</text>

  <!-- 超市 -->
  <circle cx="950" cy="280" r="22" class="node node-visited"/>
  <text x="950" y="285" class="label" fill="#263238" font-weight="bold">24</text>
  <text x="950" y="255" class="label" font-size="10">超市</text>
  <text x="950" y="315" class="dist">dist=440</text>

  <!-- 宿舍16-20 -->
  <circle cx="950" cy="450" r="22" class="node node-end"/>
  <text x="950" y="455" class="label" fill="white" font-weight="bold">42</text>
  <text x="950" y="425" class="label" font-size="10">宿舍16-20</text>
  <text x="950" y="485" class="dist">dist=560</text>

  <!-- 其他节点 -->
  <circle cx="350" cy="350" r="18" class="node node-unvisited"/>
  <text x="350" y="355" class="label" font-size="10">10</text>
  <text x="350" y="380" class="dist" font-size="9">dist=190</text>

  <circle cx="550" cy="350" r="18" class="node node-unvisited"/>
  <text x="550" y="355" class="label" font-size="10">16</text>
  <text x="550" y="380" class="dist" font-size="9">dist=320</text>

  <circle cx="750" cy="350" r="18" class="node node-unvisited"/>
  <text x="750" y="355" class="label" font-size="10">18</text>
  <text x="750" y="380" class="dist" font-size="9">dist=460</text>

  <!-- 边 -->
  <line x1="172" y1="180" x2="328" y2="180" class="edge-path"/>
  <text x="250" y="172" font-size="10" fill="#4CAF50" text-anchor="middle">80</text>

  <line x1="372" y1="180" x2="528" y2="180" class="edge-path"/>
  <text x="450" y="172" font-size="10" fill="#4CAF50" text-anchor="middle">120</text>

  <line x1="572" y1="180" x2="728" y2="180" class="edge-path"/>
  <text x="650" y="172" font-size="10" fill="#4CAF50" text-anchor="middle">130</text>

  <line x1="750" y1="202" x2="950" y2="258" class="edge-path"/>
  <text x="850" y="225" font-size="10" fill="#4CAF50" text-anchor="middle">110</text>

  <line x1="950" y1="302" x2="950" y2="428" class="edge-path"/>
  <text x="965" y="365" font-size="10" fill="#4CAF50">120</text>

  <!-- 非路径边 -->
  <line x1="350" y1="202" x2="350" y2="332" class="edge"/>
  <line x1="550" y1="202" x2="550" y2="332" class="edge"/>
  <line x1="750" y1="202" x2="750" y2="332" class="edge"/>
  <line x1="368" y1="350" x2="532" y2="350" class="edge"/>
  <line x1="568" y1="350" x2="732" y2="350" class="edge"/>

  <!-- 步骤说明 -->
  <rect x="80" y="550" width="1040" height="220" fill="white" stroke="#BDBDBD" stroke-width="1" rx="8"/>
  <text x="100" y="580" class="step" font-weight="bold">算法执行步骤：</text>

  <text x="100" y="608" class="step" font-size="12">① 初始化：dist[0]=0, dist[其他]=∞, 优先队列 pq={(0, 北门)}</text>
  <text x="100" y="632" class="step" font-size="12">② 出队 (0, 北门)，松弛邻居：dist[7]=80, dist[7]入队。pq={(80, 教学楼1)}</text>
  <text x="100" y="656" class="step" font-size="12">③ 出队 (80, 教学楼1)，松弛邻居：dist[11]=200, dist[10]=190。pq={(190, 理学院), (200, 材料学院)}</text>
  <text x="100" y="680" class="step" font-size="12">④ 出队 (190, 理学院)，松弛邻居：dist[16]=320, 但 dist[11]已更优(200)，跳过</text>
  <text x="100" y="704" class="step" font-size="12">⑤ 出队 (200, 材料学院)，松弛邻居：dist[17]=330, dist[16]已更优(320)</text>
  <text x="100" y="728" class="step" font-size="12">⑥ 出队 (330, 环安学院)，松弛邻居：dist[24]=440, dist[18]=460</text>
  <text x="100" y="752" class="step" font-size="12">⑦ 出队 (440, 超市)，松弛邻居：dist[42]=560 → 到达终点！prev[42]=24, 回溯路径：0→7→11→17→24→42</text>

  <!-- 图例 -->
  <rect x="800" y="90" width="320" height="100" fill="white" stroke="#BDBDBD" rx="6"/>
  <text x="820" y="112" font-size="12" font-weight="bold" fill="#37474F">图例：</text>
  <circle cx="830" cy="132" r="8" class="node node-start"/><text x="850" y="136" font-size="11" fill="#37474F">起点 (dist=0)</text>
  <circle cx="950" cy="132" r="8" class="node node-visited"/><text x="970" y="136" font-size="11" fill="#37474F">已确定</text>
  <circle cx="1050" cy="132" r="8" class="node node-unvisited"/><text x="1070" y="136" font-size="11" fill="#37474F">未访问</text>
  <line x1="822" y1="155" x2="840" y2="155" class="edge-path"/><text x="850" y="159" font-size="11" fill="#37474F">最短路径</text>
  <line x1="942" y1="155" x2="960" y2="155" class="edge"/><text x="970" y="159" font-size="11" fill="#37474F">普通边</text>
  <circle cx="1050" cy="155" r="8" class="node node-end"/><text x="1070" y="159" font-size="11" fill="#37474F">终点</text>
</svg>
'''

# ============================================================
#  路网数据结构图
# ============================================================
ROAD_NETWORK_DIAGRAM = r'''
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 800" font-family="Microsoft YaHei,SimHei,sans-serif">
  <defs>
    <style>
      .title { font-size: 20px; font-weight: bold; text-anchor: middle; fill: #263238; }
      .box { fill: white; stroke: #546E7A; stroke-width: 1.5; rx: 6; }
      .box-title { font-size: 14px; font-weight: bold; fill: #1565C0; }
      .code { font-family: Consolas, monospace; font-size: 11px; fill: #37474F; }
      .label { font-size: 12px; fill: #37474F; }
      .arrow { stroke: #546E7A; stroke-width: 2; fill: none; }
    </style>
    <marker id="ar" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto">
      <path d="M0,0 L9,5 L0,10 Z" fill="#546E7A"/>
    </marker>
  </defs>
  <rect width="1200" height="800" fill="#FAFAFA"/>
  <text x="600" y="35" class="title">路网构建与数据存储方案</text>

  <!-- 左侧：邻接表结构 -->
  <rect x="40" y="60" width="360" height="340" class="box"/>
  <text x="60" y="88" class="box-title">① 邻接表存储结构 (Graph类)</text>
  <text x="60" y="110" class="code">unordered_map&lt;int, Building&gt; buildings_;</text>
  <text x="60" y="125" class="code">unordered_map&lt;int, vector&lt;Edge&gt;&gt; adjacency_;</text>
  <text x="60" y="150" class="label" fill="#78909C">内存中的图结构：</text>

  <!-- 邻接表示意 -->
  <rect x="60" y="165" width="80" height="25" fill="#E3F2FD" stroke="#1565C0" rx="4"/>
  <text x="100" y="182" class="code" text-anchor="middle">id=0 北门</text>
  <text x="145" y="182" class="code">→</text>
  <rect x="160" y="165" width="90" height="25" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="205" y="182" class="code" text-anchor="middle">[7, 80.0]</text>
  <text x="255" y="182" class="code">→ null</text>

  <rect x="60" y="195" width="80" height="25" fill="#E3F2FD" stroke="#1565C0" rx="4"/>
  <text x="100" y="212" class="code" text-anchor="middle">id=7 教1</text>
  <text x="145" y="212" class="code">→</text>
  <rect x="160" y="195" width="90" height="25" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="205" y="212" class="code" text-anchor="middle">[0, 80.0]</text>
  <text x="255" y="212" class="code">→</text>
  <rect x="270" y="195" width="90" height="25" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="315" y="212" class="code" text-anchor="middle">[8, 120.0]</text>

  <rect x="60" y="225" width="80" height="25" fill="#E3F2FD" stroke="#1565C0" rx="4"/>
  <text x="100" y="242" class="code" text-anchor="middle">id=5 图书馆</text>
  <text x="145" y="242" class="code">→</text>
  <rect x="160" y="225" width="90" height="25" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="205" y="242" class="code" text-anchor="middle">[4, 130.0]</text>
  <text x="255" y="242" class="code">→</text>
  <rect x="270" y="225" width="90" height="25" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="315" y="242" class="code" text-anchor="middle">[6, 130.0]</text>

  <text x="60" y="280" class="label" fill="#78909C">每条边 Edge = {to, weight, shelter, tree, lake}</text>
  <text x="60" y="300" class="label" fill="#78909C">• to: 目标建筑ID</text>
  <text x="60" y="318" class="label" fill="#78909C">• weight: 距离(像素，由坐标计算)</text>
  <text x="60" y="336" class="label" fill="#78909C">• shelter/tree/lake: 天气标签</text>
  <text x="60" y="360" class="label" fill="#1565C0" font-weight="bold">空间复杂度: O(V + E)</text>
  <text x="60" y="378" class="label" fill="#78909C">V=66节点, E=40×2=80有向边</text>

  <!-- 中间：SQLite存储 -->
  <rect x="430" y="60" width="360" height="340" class="box"/>
  <text x="450" y="88" class="box-title">② SQLite 数据库持久化</text>
  <text x="450" y="110" class="code">DatabaseManager «singleton»</text>
  <text x="450" y="135" class="label" fill="#78909C">buildings 表：</text>
  <rect x="450" y="145" width="320" height="90" fill="#F5F5F5" stroke="#BDBDBD" rx="4"/>
  <text x="460" y="163" class="code">id | name | x | y | type | info | hours | floors | nodeKind</text>
  <text x="460" y="180" class="code">0  | 北门 |640|50 |Gate  |学校北门|全天  |1      |1</text>
  <text x="460" y="197" class="code">5  |图书馆|390|150|Lib   |藏书215万|07:30|5      |1</text>
  <text x="460" y="214" class="code">42 |宿舍16|640|850|Dorm  |本科生公寓|全天 |6      |1</text>

  <text x="450" y="255" class="label" fill="#78909C">roads 表：</text>
  <rect x="450" y="265" width="320" height="70" fill="#F5F5F5" stroke="#BDBDBD" rx="4"/>
  <text x="460" y="283" class="code">from_id | to_id | weight</text>
  <text x="460" y="300" class="code">0       | 7     | 80.0</text>
  <text x="460" y="317" class="code">5       | 6     | 130.0</text>

  <text x="450" y="355" class="label" fill="#1565C0" font-weight="bold">首次启动：seedDefaultData() 自动建表+填种子数据</text>
  <text x="450" y="375" class="label" fill="#78909C">管理员CRUD → 立即写回SQLite · 预处理语句防注入</text>

  <!-- 右侧：种子数据 -->
  <rect x="820" y="60" width="340" height="340" class="box"/>
  <text x="840" y="88" class="box-title">③ 种子数据 (CampusData.cpp)</text>
  <text x="840" y="115" class="code">void CampusData::populate(Graph&amp; g) {</text>
  <text x="840" y="133" class="code">  // 45栋建筑 (id 0-44)</text>
  <text x="840" y="151" class="code">  g.addBuilding({0,"北门",640,50,</text>
  <text x="840" y="169" class="code">    Gate,"学校北门","全天",1});</text>
  <text x="840" y="187" class="code">  g.addBuilding({5,"图书馆",390,150,</text>
  <text x="840" y="205" class="code">    Library,"藏书215万册",...});</text>
  <text x="840" y="230" class="code">  // 21个路口节点 (id 100-120)</text>
  <text x="840" y="248" class="code">  // nodeKind=0, 不显示标签</text>
  <text x="840" y="273" class="code">  // 40条道路</text>
  <text x="840" y="291" class="code">  g.addRoad(0, 7, dist(640,50,</text>
  <text x="840" y="309" class="code">    640,130), false,0,false);</text>
  <text x="840" y="333" class="code">  g.addRoad(5, 6, dist(...),</text>
  <text x="840" y="351" class="code">    true, 0, false); //有遮棚</text>
  <text x="840" y="375" class="code">}</text>

  <!-- 底部：数据流 -->
  <rect x="40" y="430" width="1120" height="130" fill="white" stroke="#4CAF50" stroke-width="2" rx="8"/>
  <text x="60" y="458" class="box-title" fill="#2E7D32">数据加载流程</text>

  <rect x="60" y="475" width="180" height="35" fill="#E3F2FD" stroke="#1565C0" rx="4"/>
  <text x="150" y="497" class="label" text-anchor="middle">程序启动</text>
  <line x1="240" y1="492" x2="290" y2="492" class="arrow" marker-end="url(#ar)"/>

  <rect x="295" y="475" width="200" height="35" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="395" y="497" class="label" text-anchor="middle">DatabaseManager.init()</text>
  <line x1="495" y1="492" x2="545" y2="492" class="arrow" marker-end="url(#ar)"/>

  <rect x="550" y="475" width="200" height="35" fill="#E8F5E9" stroke="#4CAF50" rx="4"/>
  <text x="650" y="497" class="label" text-anchor="middle">建表 + 检查是否为空</text>
  <line x1="750" y1="492" x2="800" y2="492" class="arrow" marker-end="url(#ar)"/>

  <rect x="805" y="475" width="180" height="35" fill="#F3E5F5" stroke="#9C27B0" rx="4"/>
  <text x="895" y="497" class="label" text-anchor="middle">空则调 seedData()</text>
  <line x1="895" y1="510" x2="895" y2="530" class="arrow" marker-end="url(#ar)"/>

  <rect x="805" y="530" width="180" height="35" fill="#F3E5F5" stroke="#9C27B0" rx="4"/>
  <text x="895" y="552" class="label" text-anchor="middle">CampusData::populate()</text>
  <line x1="805" y1="547" x2="755" y2="547" class="arrow" marker-end="url(#ar)"/>

  <rect x="550" y="530" width="200" height="35" fill="#E8F5E9" stroke="#4CAF50" rx="4"/>
  <text x="650" y="552" class="label" text-anchor="middle">loadAll → Graph 邻接表</text>
  <line x1="550" y1="547" x2="500" y2="547" class="arrow" marker-end="url(#ar)"/>

  <rect x="295" y="530" width="200" height="35" fill="#FFF3E0" stroke="#FF9800" rx="4"/>
  <text x="395" y="552" class="label" text-anchor="middle">MapScene.loadFromGraph()</text>
  <line x1="295" y1="547" x2="245" y2="547" class="arrow" marker-end="url(#ar)"/>

  <rect x="60" y="530" width="180" height="35" fill="#E3F2FD" stroke="#1565C0" rx="4"/>
  <text x="150" y="552" class="label" text-anchor="middle">界面显示地图</text>

  <!-- 路网布局说明 -->
  <rect x="40" y="580" width="1120" height="190" fill="white" stroke="#BDBDBD" stroke-width="1" rx="8"/>
  <text x="60" y="608" class="box-title">路网布局（基于 tjut_map.jpg 1280×976 真实底图）</text>

  <rect x="60" y="625" width="340" height="130" fill="#F5F5F5" stroke="#BDBDBD" rx="4"/>
  <text x="75" y="648" class="label" font-weight="bold" fill="#37474F">建筑布局（6排网格）：</text>
  <text x="75" y="668" class="label" fill="#546E7A">• 第1排 y≈130：教学楼群（8栋）</text>
  <text x="75" y="688" class="label" fill="#546E7A">• 第2排 y≈250：学院群（4栋）</text>
  <text x="75" y="708" class="label" fill="#546E7A">• 第3排 y≈380：学院群（7栋）</text>
  <text x="75" y="728" class="label" fill="#546E7A">• 第4排 y≈500：文科学院（4栋）</text>
  <text x="75" y="748" class="label" fill="#546E7A">• 宿舍区 y≈820-850（6栋）</text>

  <rect x="420" y="625" width="340" height="130" fill="#F5F5F5" stroke="#BDBDBD" rx="4"/>
  <text x="435" y="648" class="label" font-weight="bold" fill="#37474F">道路设计：</text>
  <text x="435" y="668" class="label" fill="#546E7A">• 大成路：x≈640 南北主干道</text>
  <text x="435" y="688" class="label" fill="#546E7A">• 知行道：y≈850 东西主干道</text>
  <text x="435" y="708" class="label" fill="#546E7A">• 横向道路：y=440, y=660</text>
  <text x="435" y="728" class="label" fill="#546E7A">• 21个路口节点(id 100-120)</text>
  <text x="435" y="748" class="label" fill="#546E7A">• 距离由坐标欧氏距离计算</text>

  <rect x="780" y="625" width="360" height="130" fill="#F5F5F5" stroke="#BDBDBD" rx="4"/>
  <text x="795" y="648" class="label" font-weight="bold" fill="#37474F">天气标签（创新功能）：</text>
  <text x="795" y="668" class="label" fill="#546E7A">• shelter=true：有连廊遮雨棚</text>
  <text x="795" y="688" class="label" fill="#546E7A">• tree=0-3：树荫等级</text>
  <text x="795" y="708" class="label" fill="#546E7A">• lake=true：临湖大风区</text>
  <text x="795" y="728" class="label" fill="#546E7A">→ dijkstraWithWeather() 根据</text>
  <text x="795" y="748" class="label" fill="#546E7A">  天气模式动态调整边权重</text>
</svg>
'''


def strip_markers(svg_str):
    """移除 SVG 中的 marker 定义和引用，改用简单线条（确保 cairosvg 不崩溃）"""
    # 移除 defs 中的 marker 元素
    svg_str = re.sub(r'<marker\b[^>]*>.*?</marker>', '', svg_str, flags=re.DOTALL)
    # 移除 marker-start/mid/end 属性
    svg_str = re.sub(r'\s*marker-(?:start|mid|end)="[^"]*"', '', svg_str)
    return svg_str

def save_svg_png(svg_str, name):
    svg_path = os.path.join(OUT_DIR, name + ".svg")
    png_path = os.path.join(OUT_DIR, name + ".png")
    with open(svg_path, "w", encoding="utf-8") as f:
        f.write(svg_str)
    try:
        cairosvg.svg2png(bytestring=svg_str.encode("utf-8"), write_to=png_path, output_width=1600)
    except Exception as e:
        print(f"  cairosvg 失败({e}), 去除marker后重试...")
        stripped = strip_markers(svg_str)
        cairosvg.svg2png(bytestring=stripped.encode("utf-8"), write_to=png_path, output_width=1600)
    print(f"已生成: {name}.png")

def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    save_svg_png(UML_CLASS_DIAGRAM, "uml_class_diagram")
    save_svg_png(USE_CASE_DIAGRAM, "use_case_diagram")
    save_svg_png(ARCH_DIAGRAM, "architecture_diagram")
    save_svg_png(DIJKSTRA_VIZ, "dijkstra_visualization")
    save_svg_png(ROAD_NETWORK_DIAGRAM, "road_network_diagram")
    print("全部图表生成完成！")

if __name__ == "__main__":
    main()
