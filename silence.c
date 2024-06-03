#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdint.h>
#include <math.h>

typedef struct {
    char        _riff_hdr[22];
    uint16_t    chan;
    uint32_t    freq, _bps;
    uint16_t    bpf, bits;  /* bytes per frame, bits per sample */
    char        _data_hdr[8];
} WAV;

static PyObject* silence_detect(PyObject *self, PyObject *args, PyObject *kw) {
    const char *fname;
    float th=-40, len=0.5;
    char *kwords[] = {"fname", "thresh", "dur", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kw, "s|ff", kwords, &fname, &th, &len))
        return NULL;
    PyObject *list = PyList_New(0);
    if (!list)
        return NULL;
    
    #define list_append(x,y) PyList_Append(list, Py_BuildValue("ff", (x), (y)))

        FILE *wave_file = fopen(fname, "r");
        if (!wave_file) {
            PyErr_SetFromErrno(PyExc_Exception);
            return NULL;
        }
        
        WAV w;
        size_t r = fread(&w, 1, sizeof(WAV), wave_file);
        if (strncmp(w._riff_hdr, "RIFF", 4) || r < sizeof(WAV)) {
            PyErr_SetString(PyExc_Exception, "Only little-endian RIFF files (WAV) are supported.");
            return NULL;
        }
        if (w.bits != 16) {
            PyErr_SetString(PyExc_Exception, "Only 16-bit samples are supported currently.");
            return NULL;
        }
        
        int32_t thresh = (1<<w.bits) * pow(10, th / 20);
        int32_t length = w.freq * len * w.chan; /* in samples */
        int n_frames = 1024*1024 / w.bpf;
        int16_t *samples = calloc(n_frames, w.bpf); /* 16-bit only */
        size_t n, frames_cnt = 0, silence_cnt = 0;
        float t0 = -1;
        
        while ((n = fread(samples, w.bpf, n_frames, wave_file))) {
            int n_samples = n * w.chan;
            for (int i=0; i<n_samples; i++) {
                if (abs(samples[i]) <= thresh) {
                    if (t0 < 0)
                        t0 = (frames_cnt + (float)i / w.chan) / w.freq; 
                    silence_cnt += 1;
                } else {
                    if (silence_cnt >= length)
                        list_append(t0, (frames_cnt + (float)i / w.chan) / w.freq);
                    silence_cnt = 0;
                    t0 = -1;
                }
            }
            frames_cnt += n;
        }
        if (silence_cnt >= length)
            list_append(t0, (float)frames_cnt / w.freq);
        fclose(wave_file);
        
    return list;
}

static PyMethodDef methods[] = {
    {"detect",  (PyCFunction)silence_detect, METH_VARARGS | METH_KEYWORDS, 
     "detect(fname, thresh=-40.0, dur=0.5)\n\n"
     "Detect silence in a WAV file.\n\n"
     "Threshold is in dBfs; silence duration is in seconds\n" 
     "Returns list of (start, end) tuples of silence segments." },
    {NULL, NULL, 0, NULL}
};
static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT, "silence", NULL, -1, methods
};
PyMODINIT_FUNC PyInit_silence(void) {
    return PyModule_Create(&module_def);
}
