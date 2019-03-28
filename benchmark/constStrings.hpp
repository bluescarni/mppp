
namespace
{
std::string const initRuntime =  "\nInit runtime :     ";
std::string const operRuntime =  "\nOperation runtime: ";
std::string const totalRuntime = "\nTotal runtime:     ";
std::string const convRuntime =  "\nConvert runtime:   ";
std::string const sortRuntime =  "\nSorting runtime:   ";

std::string const pyPrefix = "# -*- coding: utf-8 -*-\n"
                             "def get_data():\n"
                             "    import pandas\n"
                             "    data = [";

std::string const pySuffix = "]\n"
                             "    retval = pandas.DataFrame(data)\n"
                             "    retval.columns = ['Library','Task','Runtime (ms)']\n"
                             "    return retval\n\n"
                             "if __name__ == '__main__':\n"
                             "    import matplotlib as mpl\n"
                             "    mpl.use('Agg')\n"
                             "    from matplotlib.pyplot import legend\n"
                             "    import seaborn as sns\n"
                             "    df = get_data()\n"
                             "    g = sns.catplot(x='Library', y = 'Runtime (ms)', hue='Task', data=df, kind='bar', palette='muted', "
                             "legend = False, height = 5.5, aspect = 1.5)\n"
                             "    for p in g.ax.patches:\n"
                             "        height = p.get_height()\n"
                             "        g.ax.text(p.get_x()+p.get_width()/2., height + 8, '{}'.format(int(height)), "
                             "ha=\"center\", fontsize=9)\n"
                             "    legend(loc='upper left')\n"
                             "    g.fig.suptitle('%1%')\n"
                             "    g.savefig('%1%.png', bbox_inches='tight', dpi=150)\n";

std::string const bench_mpp = "\nBenchmarking mp++.";
std::string const bench_cpp_int = "\n\nBenchmarking cpp_int.";
std::string const bench_mpz_int = "\n\nBenchmarking mpz_int.";
std::string const bench_fmpzxx =  "\n\nBenchmarking fmpzxx.";
} // namespace
