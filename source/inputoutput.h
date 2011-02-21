////////////////////////////////////////////////////////////////////////////////
//
//   Copyright 2011 Mario Negri Institute & Orobix Srl
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include <QProcess>
#include <QObject>
#include <QtXml/QDomDocument>

class Graph;
class GraphLayout;
class GraphProperties;
class GraphMesh;
class QFile;

class InputOutput : public QObject
{
    Q_OBJECT
public:
    explicit InputOutput(QObject* parent = 0);

    bool loadGraphFromLayout(Graph* graph, GraphLayout* graphLayout, GraphProperties* graphProperties);
    bool loadGraphFromGraph(Graph* graph, GraphLayout* graphLayout, GraphProperties* graphProperties);

    void loadMesh(GraphMesh* graphMesh);

    void saveGraph(const QString& fileName, GraphProperties* graphProperties, QVector<int> nodes, QVector<int> edges);
    void saveNetwork(const QString& fileName, GraphLayout* graphLayout, GraphProperties* graphProperties, QVector<int> nodes, QVector<int> edges);

    void generateMesh(const QString& fileName);
    void loadMeshAfterGenerating(const QString& fileName, GraphMesh* graphMesh);

    void customizeGraph(const QString& fileName);

signals:
    void curFileName(QString theName);
    void graphSaved(QString theName);
    void pyNSCompleted(QString fileName);
    void meshFileReady(QString fileName);

public slots:
    void errorFromExternal(QProcess::ProcessError);
    void meshingComplete();

private:
    void loadGraph(QDomDocument theDomDoc, Graph *graph, GraphProperties *graphProperties);
    void loadLayout(QDomDocument theDomDoc, GraphLayout* graphLayout);
    void showWarningMessage(QString theMessage);

    QProcess* pyNS;

    QString meshOut;
};

#endif // INPUTOUTPUT_H
