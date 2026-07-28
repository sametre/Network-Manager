using System;
using System.Windows.Forms;

namespace ReHargi.PosNetworkManager.Forms
{
    partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            this.Text = "Re Hargi – POS Network Manager";
        }
    }
}
