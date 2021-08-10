


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void PlotStart(FILE *pf, const char *title)
{
    fprintf(pf,
"%%-------------------------------------------------------------------------------\n"
"%%-------------------------------------------------------------------------------\n"
"\\begin{semilogyaxis}[\n"
"    grid = both,\n"
"    name = myaxis,\n"
"    y label style={font=\\footnotesize},\n"
"    ylabel = timings (ms),\n"
"    x tick label style={font=\\footnotesize},\n"
"    symbolic x coords={1, 2, 3, 4, 5, 6, 7},\n"
"    xtick = data,\n"
"    enlarge x limits = {abs=0.85cm},\n"
"    title = %s,\n"
"    legend style={at={(0.01, 0.98), fill = mygreylighter, rounded corners = 0.8},\n"
"    nodes near coords,\n"
"    every node near coord/.append style={font=\\tiny},\n"
"    anchor = north west, legend columns=-1, append after command={\n"
"        \\pgfextra{\n"
"          \\draw[rounded corners = 0.8, drop shadow, fill opacity = 0.0]\n"
"            (\\tikzlastnode.south west)rectangle(\\tikzlastnode.north east);\n"
"          }\n"
"       }}\n"
"]\n",
            title
    );
}

void
AddPlot(FILE *pf, const char *color, const char *fillColor, const float *data, bool nodeNorth)
{
fprintf(pf,
"\\addplot[%spoint meta=rawy, mark=*, color=%s, mark options={solid, fill=%s}, line width = 0.6pt] coordinates {\n"
"    (1, %f)\n"
"    (2, %f)\n"
"    (3, %f)\n"
"    (4, %f)\n"
"    (5, %f)\n"
"    (6, %f)\n"
//"    (7, %f)\n"
"};\n",
        nodeNorth ? "nodes near coords align={anchor=north}, " : "",
        color,
        fillColor,
        data[0],
        data[1],
        data[2],
        data[3],
        data[4],
				data[5]//,
//				data[6]
        );
}

void PlotStop(FILE *pf)
{
fprintf(pf,
"\\legend{\n"
"    \\tiny{CPU\\_1},\n"
"    \\tiny{CPU\\_2},\n"
"    \\tiny{CPU\\_4},\n"
"    \\tiny{CPU\\_8},\n"
"    \\tiny{CPU\\_16},\n"
"    \\tiny{CPU\\_32},\n"
"}\n"
"\\end{semilogyaxis}\n"
"\n"
"\\begin{pgfonlayer}{background}\n"
"\\draw[fill=mygreylighterr, drop shadow]\n"
"        (myaxis.north west)\n"
"        rectangle (myaxis.south east);\n"
"\\end{pgfonlayer}\n"
"%%-------------------------------------------------------------------------------\n"
"%%-------------------------------------------------------------------------------\n"
"\n"
"\\node[black, draw=none, anchor = east] at (0.8, -0.22) {\\scalebox{0.9}{{depth: }}};\n"
);
}

void
CreatePlot(const char *meshName, const char *performanceType, const char *plotLabel)
{
    const char *colors[] = {
        "myorange",
        "mygreen",
        "myred",
        "myblue",
        "mygrey",
        "myblack"
    };
    const char *fillColors[] = {
        "myorangelight",
        "mygreenlight",
        "myredlight",
        "mybluelight",
        "mygreylight",
        "myblack"
		};
    const int32_t threadCount = 6;
    const int32_t maxDepth = 6;
    float data[maxDepth];
    char buffer[256];
    FILE *pf;

    sprintf(buffer, "cpu-%s-%s.tex", meshName, performanceType);
    pf = fopen(buffer, "w");
    PlotStart(pf, plotLabel);

    // read numbers
    for (int32_t threadID = 0; threadID < threadCount; ++threadID) {
        const int32_t nthreads = 1 << threadID ;
        for (int32_t depth = 1; depth <= maxDepth; ++depth) {
            FILE *dataFile;

            sprintf(buffer,
                    "%s_%s_%i_%i.txt",
                    meshName,
                    performanceType,
                    depth,
                    nthreads);
            printf("Opening %s\n",buffer) ;
            dataFile = fopen(buffer, "r");
            fscanf(dataFile, "%f", &data[depth - 1]);
            fclose(dataFile);
        }

        AddPlot(pf,
                colors[threadID],
                fillColors[threadID],
                data,
                threadID == threadCount - 1);
    }


    PlotStop(pf);
    fclose(pf);
}

int main(int argc, char **argv)
{
    const int32_t meshCount = argc - 1;

    for (int32_t meshID = 0; meshID < meshCount; ++meshID) {
        char meshName[64];
        const char *filename = argv[meshID + 1];
        const char *preFix = strrchr(filename, '/') + 1;
        const char *postFix = strrchr(filename, '.');

        memset(meshName, 0, sizeof(meshName));
        if (preFix != NULL) {
            if (postFix != NULL) {
                memcpy(meshName, preFix, postFix - preFix);
            } else {
                memcpy(meshName, preFix, strlen(preFix));
            }
        } else {
            if (postFix != NULL) {
                memcpy(meshName, filename, filename - postFix);
            } else {
                memcpy(meshName, filename, strlen(filename));
            }
        }

        CreatePlot(meshName, "halfedge", "Halfedge Refinement");
        CreatePlot(meshName, "crease", "Crease Refinement");
        CreatePlot(meshName, "points", "Vertex-Point Refinement");

        printf("%s (%s)\n", meshName, filename);
    }

    return 0;
}
