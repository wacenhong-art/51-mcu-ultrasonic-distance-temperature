from pathlib import Path
import pymupdf as fitz
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.utils import ImageReader
root=Path(__file__).resolve().parents[1]
pdfmetrics.registerFont(TTFont('Song',r'C:/Windows/Fonts/simsun.ttc',subfontIndex=0))
W,H=595.276,841.89
supp=root/'tmp/pdfs/补充页.pdf';c=canvas.Canvas(str(supp),pagesize=(W,H))
def text(x,y,s,size=10.5):
 c.setFont('Song',size);c.drawString(x,y,s)
def line(y,s,size=10.5): text(48,y,s,size)
def title(s):
 text(48,792,s,16);c.setLineWidth(.7);c.line(48,778,547,778)
def footer(s):text(48,31,s,9)
title('实验三  实验报告填写页（精简）')
line(754,'姓名：____________  学号：____________  班级：____________')
line(734,'实验日期：2026年11月16日')
line(706,'一、实际接线与显示',12)
line(685,'HC-SR04：Trig接P1.0，Echo接P1.1，电源接5V与GND。')
line(666,'DS18B20使用板载P3.7插座；左四位显示距离，右四位显示温度。')
line(647,'K1锁定距离，L为锁定标志；距离小于10cm时蜂鸣报警。')
line(619,'二、测距流程图',12)
# Two rows of four nodes, follow arrows left to right.
labels=[['触发超声波','等待回波上升','记录T1起点','等待回波下降'],['计算T1差值','温度修正声速','计算距离','显示并判断报警']]
for row,items in enumerate(labels):
 y=580-row*49
 for col,label in enumerate(items):
  x=48+col*128;c.rect(x,y-12,115,29);text(x+6,y-1,label,10)
  if col<3:
   c.line(x+116,y+2,x+126,y+2);c.line(x+122,y+5,x+126,y+2);c.line(x+122,y-1,x+126,y+2)
c.line(547,582,553,582);c.line(553,582,553,557);c.line(553,557,42,557);c.line(42,557,42,533);c.line(42,533,48,533)
line(500,'等待回波超时则显示“---”并关闭报警；各轮间隔约80ms。',10)
line(473,'三、双定时器配置与计算',12)
line(451,'TMOD=11H：T0、T1均为16位定时方式1；GATE=0，C/T=0。')
line(432,'计数频率=11.0592MHz/12=921600Hz，计数周期约1.085μs。')
line(413,'T0：初值65536-922=FC66H；开中断，约1ms扫描一位。')
line(394,'八位整屏刷新约125Hz。T1：初值0000H，自由计数，不开中断。')
line(375,'记录回波两沿计数差N，相当于测量脉宽；30ms超时避免长时间等待。')
line(356,'温度补偿：v=331.4+0.6T（m/s）；d=100vN/(2×921600)（cm）。')
line(328,'四、测量结果与误差分析',12)
headers=['标尺/cm','显示/cm','温度/℃','绝对误差/cm','相对误差']
widths=[90,90,90,115,114];xx=[48]
for w in widths:xx.append(xx[-1]+w)
rows=[headers,['5','5.4','19.1','0.4','8.0%'],['10','10.4','19.4','0.4','4.0%'],['20','20.2','19.5','0.2','1.0%'],['40','40.4','20.0','0.4','1.0%']]
for i,row in enumerate(rows):
 y=307-i*25
 for j,s in enumerate(row):c.rect(xx[j],y-25,widths[j],25);text(xx[j]+8,y-17,s,10)
line(161,'绝对误差=|显示值-标尺值|；相对误差=绝对误差/标尺值×100%。',10)
line(142,'平均绝对误差0.35cm，最大0.4cm；偏差可能来自基准面、反射角和检测延迟。',10)
line(123,'按老师要求记录四组。温度为记录时显示值，距离锁定后温度仍会更新。',10)
line(104,'已实现温度补偿；未采集补偿前对照数据，因此不计算误差改善比例。',10)
line(85,'实验结论：测距、温度显示与蜂鸣功能正常，完成四组距离测量。',10)
footer('补充填写页 1/2  |  前四页保留原指导书；实际改动以本页记录为准。')
c.showPage();title('实验三  实物照片与运行记录')
line(751,'运行结果：左侧显示距离，右侧显示温度；实测数据见前页。')
img=ImageReader(str(root/'evidence/实物运行照片.jpg'));iw,ih=img.getSize();scale=min(470/iw,565/ih);ww,hh=iw*scale,ih*scale
c.drawImage(img,(W-ww)/2,158,width=ww,height=hh)
line(135,'照片显示约21.4cm、20.7℃，为独立运行画面，不对应表中四组数据。',10)
line(115,'蜂鸣正常依据实际操作反馈；静态照片仅记录接线和显示状态。',10)
line(95,'调试记录：修正DS18B20读取时序后温度恢复显示；程序编译无错误、无警告。',10)
footer('补充填写页 2/2  |  原始照片与程序、实测记录一并归档。');c.save()
src=fitz.open(root/'docs/实验三_指导书原页.pdf');out=fitz.open();out.insert_pdf(src);extra=fitz.open(supp);out.insert_pdf(extra)
path=root/'output/pdf/实验三_指导书原页实验报告_精简答案.pdf';out.set_metadata({'title':'实验三：基于超声波测距的智能报警与距离显示系统实验报告','subject':'原指导书四页、精简答案、四组实测数据与实物照片'});out.save(path,garbage=4,deflate=True)
check=fitz.open(path);assert len(check)==6
for i in range(4):assert src[i].get_pixmap().samples==check[i].get_pixmap().samples
for i,p in enumerate(check):p.get_pixmap(matrix=fitz.Matrix(1.2,1.2)).save(root/f'tmp/pdfs/report-{i+1}.png')
assert '0.35cm' in check[4].get_text();print(path.resolve());print('6 pages; original four pages pixel-identical.')
