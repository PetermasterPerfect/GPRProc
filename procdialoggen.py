from procdialoghdr import *
from procdialogsrc import *
from collections import namedtuple

class In():
    n = 0
    def __init__(self, ty, label, **kwargs):
        self.ty = ty
        self.label = label
        self.name = f"input{In.n}"
        In.n+=1
        self.rang3 = kwargs.get("rang3")
        self.value = kwargs.get("value")
        self.mn = kwargs.get("mn")
        self.mx = kwargs.get("mx")
        self.single_step = kwargs.get("single_step")
        self.decimals = kwargs.get("decimals")


    def gen_input(self):
        ret = f"auto {self.name} = new {self.ty};\n"
        if self.rang3:
            ret += f"{self.name}->setRange({self.rang3});\n"
        if self.value:
            ret += f"{self.name}->setValue({self.value});\n"
        if self.mn:
            ret += f"{self.name}->setMinimum({self.mn});\n"
        if self.mx:
            ret += f"{self.name}->setMaximum({self.mx});\n"
        if self.single_step:
            ret += f"{self.name}->setSingleStep({self.single_step});\n"
        if self.decimals:
            ret += f"{self.name}->setDecimals({self.decimals});\n"
        return ret


class Procedur():
    def __init__(self, label, dialog_f, prof_f):
        self.label = label
        self.dialog_func = dialog_f
        self.profile_func = prof_f
        self.prototype = self.find_prototype()
        self.extra = ''
        self.inputs = []


    def find_prototype(self):
        with open('tags', 'r') as f:
            dt = f.read()
        start = dt.find(self.profile_func+'(')+len(self.profile_func+'(')
        end = dt.find(')', start)
        return dt[start:end]


class Template():
    def __init__(self, proc):
        self.procedures = proc
        self.create_page_names = ['create'+x.dialog_func.title() for x in self.procedures]
        self.slots = ['on'+x.dialog_func.title() for x in self.procedures]


    def gen_hdr(self):
        proc_names = '{"'+'", "'.join([x.label for x in self.procedures])+'"}'
        pages = [f'QWidget* {x}()' for x in self.create_page_names]
        slots_dec = '\n\t'.join([f'void {x}(bool checked);' for x in self.slots])
        slots_func = ',\n\t'.join([f'\t&ProceduresDialog::{x}' for x in self.slots])
        pages_dec = '\n\t'.join([x+';' for x in pages]) 
        return CLASS_DEF.format(proc_names, slots_dec, len(self.procedures), slots_func, pages_dec) 


    def gen_src(self):
        created_pages = [f'auto {self.procedures[x].dialog_func}Page = {self.create_page_names[x]}();' for x in range(len(self.procedures))]
        stack_add = [f'stack->addWidget({x.dialog_func}Page);' for x in self.procedures];
        on_proc = []
        for i in range(len(self.procedures)):
            on_proc.append(ON_PROCEDUR.format(self.slots[i], i, self.procedures[i].profile_func))

        create_pages_func = []
        for p in self.procedures:
            create_pages_func.append(self.gen_create_page(p))

        src = INIT.format('\n\t'.join(created_pages), '\n\t'.join(stack_add)) + ''.join(on_proc) + '\n'.join(create_pages_func)
        return src
    

    def gen_create_page(self, proc):
        name = 'create'+proc.dialog_func.title()
        setup = '\t\n'.join([x.gen_input() for x in proc.inputs])
        
        layouts = ''
        for i, e in enumerate(proc.inputs):
            layouts += f"""auto hLayout1_{i} = new QHBoxLayout;
    hLayout1_{i}->addWidget(new QLabel("{e.label}"));
    hLayout1_{i}->addWidget({e.name});
    layout->addLayout(hLayout1_{i});\n"""
        args_pass = ', '.join([f'{x.name}->value()' for x in proc.inputs])
        return CREATE_PAGE.format(name, proc.label, setup, layouts, proc.prototype, args_pass, proc.prototype, args_pass, extra=proc.extra)


if __name__ == "__main__":
    dc = Procedur("Subtract DC-shift", 'dcShift', 'subtractDcShift')
    dc.inputs = [In('QDoubleSpinBox', 'Time window 1: ', value='0', rang3='0, profile->timeWindow', decimals='3', single_step='0.001'),
        In('QDoubleSpinBox', 'Time window 2: ', value='profile->timeWindow', rang3='0, profile->timeWindow', decimals='3', single_step='0.001')]

    dewow = Procedur("Subtract mean (dewow)", 'dewow', 'subtractDewow')
    dewow.inputs = [In('QDoubleSpinBox','Time window 1: ', value='0', rang3='0, profile->timeWindow', decimals='3', single_step='0.001')]

    gain = Procedur("Exponent gain", 'gain', 'gainFunction')
    gain.inputs = [
            In('QDoubleSpinBox','Start time: ', value='0', rang3='0, profile->timeWindow', decimals='3', single_step='0.001'),
            In('QDoubleSpinBox','End time: ', value='profile->timeWindow', rang3='0, profile->timeWindow', decimals='3', single_step='0.001'),
            In('QDoubleSpinBox','Exponent: ', value='0', mn='0', decimals='3', single_step='0.001'),
            In('QSpinBox', 'Max value: ', value='100000', rang3='0, 100000000')]


    ampl0 = Procedur("Amplitudes to 0", 'amplitudesTo0', 'ampltitudesTo0')
    ampl0.extra = """
        float maxVal = profile->maxAmplitude();
        float minVal = profile->minAmplitude();
        float range = sqrt(maxVal*maxVal-minVal*minVal);
    """
    ampl0.inputs = [
            In('QDoubleSpinBox','Min amplitude: ', value='minVal', rang3='minVal, maxVal', decimals='3', single_step='range/1000'),
            In('QDoubleSpinBox','Max amplitude: ', value='maxVal', rang3='minVal, maxVal', decimals='3', single_step='range/1000')]

    xflip = Procedur("X(traces) flip", 'xFlip', 'xFlip')
    yflip = Procedur("Y(samples) flip", 'yFlip', 'yFlip')
    time_cut = Procedur("Time cut", 'timeCut', 'timeCut')
    time_cut.inputs = [
            In('QDoubleSpinBox','Time to cut: ', value='profile->timeWindow', rang3='0, profile->timeWindow*10', decimals='3', single_step='profile->timeWindow/profile->samples')]

    move_start = Procedur("Move start time", 'moveStartTime', 'moveStartTime')
    move_start.inputs = [
            In('QDoubleSpinBox','Start time: ', value='0', rang3='-1*profile->timeWindow+profile->timeWindow/profile->samples, profile->timeWindow-profile->timeWindow/profile->samples', decimals='3', single_step='profile->timeWindow/profile->samples')]


    butterworth = Procedur("Butterworth filter", 'butterworthFilter', 'butterworthFilter')
    butterworth.inputs = [
            In('QDoubleSpinBox','Low cutoff (MHz): ', value='0', rang3='0, profile->fs()/2-1', decimals='3', single_step='1'),
            In('QDoubleSpinBox','High cutoff (MHz): ', value='profile->fs()/1e+6/2-1', rang3='0, profile->fs()/2-1', decimals='3', single_step='1'),
            In('QDoubleSpinBox', 'Stopband attenuation [dB]: ', value='1', single_step='0.1', rang3='1, 1000'),
            In('QDoubleSpinBox', 'Passband ripple [dB]: ', value='1', single_step='0.1', rang3='1, 1000')]



    temp = Template([dc, dewow, gain, ampl0, xflip, yflip, time_cut, move_start, butterworth])
    with open('proceduresdialog.h', 'w') as f:
        f.write(temp.gen_hdr())
    with open('proceduresdialog.cpp', 'w') as f:
        f.write(temp.gen_src())
