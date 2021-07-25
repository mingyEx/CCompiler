using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Msagl.Drawing;
namespace CFGView
{
    public partial class CFGForm : Form
    {

        string ReadString(BinaryReader reader)
        {
            int len = reader.ReadInt32();
            return Encoding.Unicode.GetString(reader.ReadBytes(len * 2));
        }
        public CFGForm(string fileName)
        {
            InitializeComponent();
            if (fileName == null)
            {
                var openDlg = new OpenFileDialog(); //待会改成清楚版的...
                openDlg.Filter = "CFG Dump|*.cfgdump|All Files|*.*";
                if (openDlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                    fileName = openDlg.FileName;
            }
            if (fileName != null)
            {
                Graph g = new Graph();
                List<Node> nodes = new List<Node>();
                StringBuilder sb = new StringBuilder();
                using (var reader = new BinaryReader(new FileStream(fileName, FileMode.Open), Encoding.Unicode))
                {
                    int count = reader.ReadInt32();
                    for (int i = 0; i < count; i++)
                    {
                        Node n = new Node();
                        n.Id = reader.ReadInt32();
                        n.Text = ReadString(reader);
                        n.Text = n.Text.Replace(';', '\n');
                        n.Child1 = reader.ReadInt32();
                        n.Child2 = reader.ReadInt32();
                        int idom = reader.ReadInt32();
                        sb.Clear();
                        sb.AppendLine("Idom: " + idom.ToString());

                        //我的添加      不行，待会看看dump格式，把DOm写进去吧

                        //sb.Append("Dom: [");
                        //int Dom = reader.ReadInt32();
                        //for (int j = 0; j < Dom; j++)
                        //{
                        //    if (j > 0)
                        //        sb.Append(", ");
                        //    sb.Append(reader.ReadInt32().ToString());
                        //}
                        //sb.AppendLine("]");

                        //end
                        sb.Append("DomFrontier: [");
                        int domFronts = reader.ReadInt32();
                        for (int j = 0; j < domFronts; j++)
                        {
                            if (j > 0)
                                sb.Append(", ");
                            sb.Append(reader.ReadInt32().ToString());
                        }
                        sb.AppendLine("]");
                        sb.Append("DomChildren: [");
                        int domChildren = reader.ReadInt32();
                        for (int j = 0; j < domChildren; j++)
                        {
                            if (j > 0)
                                sb.Append(", ");
                            sb.Append(reader.ReadInt32().ToString());
                        }
                        sb.AppendLine("]");
                        sb.AppendLine("R_Idom: " + reader.ReadInt32().ToString());
                        sb.Append("R_DomFrontier: [");
                        int rdomFronts = reader.ReadInt32();
                        for (int j = 0; j < rdomFronts; j++)
                        {
                            if (j > 0)
                                sb.Append(", ");
                            sb.Append(reader.ReadInt32().ToString());
                        }
                        sb.Append("]");
                        n.Desc = sb.ToString();
                        nodes.Add(n);
                    }
                }
                foreach (var n in nodes)
                {
                    if (n.Child1 != -1)
                        g.AddEdge(n.ToString(), nodes[n.Child1].ToString());
                    if (n.Child2 != -1)
                        g.AddEdge(n.ToString(), nodes[n.Child2].ToString()).Attr.Color = new Microsoft.Msagl.Drawing.Color(0, 0, 255);
                    var gn = g.FindNode(n.ToString());
                    gn.Attr.Shape = Shape.Box;
                    gn.Attr.LabelMargin = 10;
                }

                Microsoft.Msagl.GraphViewerGdi.GViewer viewer = new Microsoft.Msagl.GraphViewerGdi.GViewer();
                viewer.Parent = this;
                viewer.Dock = DockStyle.Fill;
                viewer.Graph = g;
            }
            else
                Close();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
    }

    public class Node
    {
        public int Id;
        public string Text;
        public string Desc;
        public int Child1, Child2;
        public override string ToString()
        {
            return Id.ToString() + "\n" + Text + "\n---\n" + Desc;
        }
    }
}
