#!//usr/local/bin/python2
import math
import operator
from plotly.offline import download_plotlyjs, init_notebook_mode, plot, iplot
from plotly.graph_objs import Scatter, Figure, Layout
import plotly.plotly as py
import plotly.graph_objs as go
import pandas as pd
import numpy as np

Xmin=3
Xmax=9

for Abuse in ("Spam Domains", "Phishing Domains", "Malware Domains", "Botnet Domains"):
	InitialDataPoint = '2017-may-tlds.csv'
	DataPoints = ['2017-may-tlds.csv', '2017-june-tlds.csv', '2017-july-tlds.csv', '2017-aug-tlds.csv', '2017-sept-tlds.csv']


	# read data files
	datasets=pd.DataFrame()
	DataPointsAxes=[]
	for DataPoint in DataPoints:
		dataset=pd.read_csv(DataPoint,index_col=['TLD'])
		dataset["DataPoint"]=DataPoint
		datasets=datasets.append(dataset)
		DataPointsAxes.append(DataPoint.strip("-tlds.csv"))
	TLDs=sorted(set(list(datasets.index)))
	empty=pd.DataFrame(index=TLDs)
	empty["Domains"]=1
	empty["Spam Domains"]=0
	empty["Phishing Domains"]=0
	empty["Malware Domains"]=0
	empty["Botnet Domains"]=0

	#auto-scale
	Xnp=datasets["Domains"]
	Znp=datasets[Abuse]
	Ynp=Znp/Xnp*100
	Ymax=Ynp.max()*1.2
	Ymin=-Ymax*2/10



	# make figure
	figure = {
	    'data': [],
	    'layout': {},
	    'frames': []
	}

	# fill in most of layout
	figure['layout']['xaxis'] = {'type': 'log', 'range': [Xmin, Xmax], 'title': 'Number of Resolving Domains in TLD Zone'}
	figure['layout']['yaxis'] = { 'range': [Ymin, Ymax], 'title': 'Percent of Abuse: '+Abuse}
	figure['layout']['hovermode'] = 'closest'
	figure['layout']['showlegend'] = False
	figure['layout']['title'] = Abuse
	figure['layout']['titlefont'] = {
		'size': 48
	}
	figure['layout']['sliders'] = {
	    'args': [
		'transition', {
		    'duration': 400,
		    'easing': 'cubic-in-out'
		}
	    ],
	    'initialValue': InitialDataPoint,
	    'plotlycommand': 'animate',
	    'values': 'DataPointsAxes',
	    'visible': True
	}
	figure['layout']['updatemenus'] = [
	    {
		'buttons': [
		    {
			'args': [None, {'frame': {'duration': 500, 'redraw': False},
				 'fromcurrent': True,
							 'transition': {'duration': 300, 'easing': 'quadratic-in-out'}
								}],
			'label': 'Play',
			'method': 'animate'
		    },
		    {
			'args': [[None], {'frame': {'duration': 0, 'redraw': False}, 'mode': 'immediate',
			'transition': {'duration': 0}
						}],
			'label': 'Pause',
			'method': 'animate'
		    }
		],
		'direction': 'left',
		'pad': {'r': 10, 't': 87},
		'showactive': False,
		'type': 'buttons',
		'x': 0.1,
		'xanchor': 'right',
		'y': 0,
		'yanchor': 'top'
	    }
	]

	sliders_dict = {
	    'active': 0,
	    'yanchor': 'top',
	    'xanchor': 'left',
	    'currentvalue': {
		'font': {'size': 12},
		'prefix': 'Data Point:',
		'visible': True,
		'xanchor': 'right'
	    },
	    'transition': {'duration': 300, 'easing': 'cubic-in-out'},
	    'pad': {'b': 10, 't': 50},
	    'len': 0.9,
	    'x': 0.1,
	    'y': 0,
	    'steps': []
	}


	# make data
	dataset=datasets[datasets["DataPoint"]==InitialDataPoint]
	dataset=dataset.combine_first(empty)
	Xlist=list(dataset['Domains'])
	Zlist=list(dataset[Abuse])
	Xnp=np.array(Xlist, dtype=np.float)
	Znp=np.array(Zlist, dtype=np.float)
	Ynp=Znp/Xnp*100
	Ylist=list(Ynp)
	AvgY=np.mean(Ynp)
	AvgZ=np.mean(Znp)
	Redlist =list((np.maximum(np.zeros_like(Ynp),Ynp-AvgY)*255/(np.amax(Ynp)-AvgY)).astype(int))
	Bluelist=list((np.maximum(np.zeros_like(Znp),Znp-AvgZ)*255/(np.amax(Znp)-AvgZ)).astype(int))
	Colorlist=[]
	for color in zip(Redlist, Bluelist):
		RedShade,BlueShade=color
		Colorlist.append("\'rgb("+str(RedShade)+", 0,"+str(BlueShade)+")\'")
	for tld in zip(TLDs, Xlist, Zlist, Ylist, Colorlist):
		TLDname,X,Z,Y,Color = tld
		data_dict = {
			'x': [X],
			'y': [Y],
			'mode': 'markers',
			'text': [TLDname],
			'marker': {
				'color' : [Color],
				'sizemode': 'area',
				'sizeref': 80,
				'size': [Z]
			},
			'name': TLDname
		}
		figure['data'].append(data_dict)

	# plot average score line
	data_dict_avg = {
		'x': [math.pow(10,Xmin), math.pow(10,Xmax)],
		'y': [AvgY,AvgY],
		'line': {
			'color': 'rgb(255, 0, 0)',
			'width': 2,
			'dash': 'longdash'
		}
	}
	figure['data'].append(data_dict_avg)
	data_dict_avg_txt = {
		'x': [math.pow(10,Xmax)/3],
		'y': [AvgY+Ymax/100],
		'text': 'Average Score',
		'textposition': 'top',
		'textfont': {
			'color':'rgb(255,0,0)'
		},
		'mode': 'text'
	}
	figure['data'].append(data_dict_avg_txt)

	# plot 10x average score line
	data_dict_10xavg = {
		'x': [math.pow(10,Xmin), math.pow(10,Xmax)],
		'y': [AvgY*10,AvgY*10],
		'line': {
			'color': 'rgb(255, 0, 0)',
			'width': 2,
			'dash': 'dot'
		}
	}
	figure['data'].append(data_dict_10xavg)
	data_dict_10xavg_txt = {
		'x': [math.pow(10,Xmax)/3],
		'y': [AvgY*10+Ymax/100],
		'text': '10x Average Score',
		'textposition': 'top',
		'textfont': {
			'color':'rgb(255,0,0)'
		},
		'mode': 'text'
	}
	figure['data'].append(data_dict_10xavg_txt)

	# print average abuse text
	data_dict_avg_abuse_txt = {
		'x': [math.pow(10,Xmax)/3],
		'y': [Ymax/2-2*Ymax/100],
		'text': 'Average Abuse',
		'textposition': 'bottom',
		'textfont': {
			'color':'rgb(0,0,255)'
		},
		'mode': 'text'
	}
	figure['data'].append(data_dict_avg_abuse_txt)
	data_dict_avg_10xabuse_txt = {
		'x': [math.pow(10,Xmax)/3],
		'y': [Ymax*3/4-5*Ymax/100],
		'text': '10x Average Abuse',
		'textposition': 'bottom',
		'textfont': {
			'color':'rgb(0,0,255)'
		},
		'mode': 'text'
	}
	figure['data'].append(data_dict_avg_10xabuse_txt)

	# draw average abuse
	data_dict_abuse ={
		'x': [math.pow(10,Xmax)/3, math.pow(10,Xmax)/3],
		'y': [Ymax/2, Ymax*3/4],
		'mode': 'markers',
		'marker': {
			'sizemode': 'area',
			'sizeref': 80,
			'size': [AvgZ,10*AvgZ],
			'color': 'rgb(255,255,255)',
			'line': {
				'width': 2,
				'color': 'rgb(0,0,255)',
				'dash': 'dot'
			}
		}
	}
	figure['data'].append(data_dict_abuse)

	# draw colorbar
	data_dict_colormap={
		'x': [],
		'y': [],
		'colorscale':[[0.0, 'rgb(0,0,0)'], [(10*AvgY-AvgY)*100/(Ymax-AvgY), 'rgb('+str((10*AvgY-AvgY)*255/(Ymax-AvgY))+',0,0)'], [100, 'rgb(255,0,0)']],
		'colorbar': {
			'title': 'Distance to average',
			'titleside': 'top',
			'tickmode': 'array',
			'tickvals': [0, (10*AvgY-AvgY)*100/(Ymax-AvgY), 100],
			'ticktext': ['Avg', '10x Avg', 'Max'],
			'ticks': 'outside'
		}
	}
	figure['data'].append(data_dict_colormap)

	# make frames
	for DataPoint in DataPoints:
		frame = {'data': [], 'name': DataPoint.strip('-tlds.cvs')}
		dataset=datasets[datasets["DataPoint"]==DataPoint]
		dataset=dataset.combine_first(empty)
		Xlist=list(dataset['Domains'])
		Zlist=list(dataset[Abuse])
		Xnp=np.array(Xlist, dtype=np.float)
		Znp=np.array(Zlist, dtype=np.float)
		Ynp=Znp/Xnp*100
		Ylist=list(Ynp)
		AvgY=np.mean(Ynp)
		AvgZ=np.mean(Znp)
		Redlist=list((np.maximum(np.zeros_like(Ynp),Ynp-AvgY)*255/(np.amax(Ynp)-AvgY)).astype(int))
		Bluelist=list((np.maximum(np.zeros_like(Znp),Znp-AvgZ)*255/(np.amax(Znp)-AvgZ)).astype(int))
		Colorlist=[]
		for color in zip(Redlist, Bluelist,TLDs, Zlist):
			RedShade,BlueShade,tld,z=color
			Colorlist.append("\'rgb("+str(RedShade)+", 0,"+str(BlueShade)+")\'")
		for tld in zip(TLDs, Xlist, Zlist, Ylist, Colorlist):
			TLDname,X,Z,Y,Color = tld
			data_dict = {
				'x': [X],
				'y': [Y],
				'mode': 'markers',
				'text': [TLDname],
				'marker': {
					'color' : [Color],
					'sizemode': 'area',
					'sizeref': 80,
					'size': [Z]
				},
				'name': TLDname
			}
			frame['data'].append(data_dict)

	# plot average score
		data_dict_avg = {
			'x': [math.pow(10,Xmin), math.pow(10,Xmax)],
			'y': [AvgY,AvgY],
			'line': {
				'color': 'rgb(255, 0, 0)',
				'width': 2,
				'dash': 'longdash'
			}
		}
		frame['data'].append(data_dict_avg)
		data_dict_avg_txt = {
			'x': [math.pow(10,Xmax)/3],
			'y': [AvgY+Ymax/100],
			'text': 'Average Score',
			'textposition': 'top',
			'textfont': {
				'color':'rgb(255,0,0)'
			},
			'mode': 'text'
		}
		frame['data'].append(data_dict_avg_txt)

	# plot 10x average score
		data_dict_10xavg = {
			'x': [math.pow(10,Xmin), math.pow(10,Xmax)],
			'y': [AvgY*10,AvgY*10],
			'line': {
				'color': 'rgb(255, 0, 0)',
				'width': 2,
				'dash': 'dot'
			}
		}
		frame['data'].append(data_dict_10xavg)
		data_dict_10xavg_txt = {
			'x': [math.pow(10,Xmax)/3],
			'y': [AvgY*10+Ymax/100],
			'text': '10x Average Score',
			'textposition': 'top',
			'textfont': {
				'color':'rgb(255,0,0)'
			},
			'mode': 'text'
		}
		frame['data'].append(data_dict_10xavg_txt)

	# print average abuse text
		data_dict_avg_abuse_txt = {
			'x': [math.pow(10,Xmax)/3],
			'y': [Ymax/2-2*Ymax/100],
			'text': 'Average Abuse',
			'textposition': 'bottom',
			'textfont': {
				'color':'rgb(0,0,255)'
			},
			'mode': 'text'
		}
		frame['data'].append(data_dict_avg_abuse_txt)
		data_dict_avg_10xabuse_txt = {
			'x': [math.pow(10,Xmax)/3],
			'y': [Ymax*3/4-5*Ymax/100],
			'text': '10x Average Abuse',
			'textposition': 'bottom',
			'textfont': {
				'color':'rgb(0,0,255)'
			},
			'mode': 'text'
		}
		frame['data'].append(data_dict_avg_10xabuse_txt)

	# draw average abuse
		data_dict_abuse ={
			'x': [math.pow(10,Xmax)/3, math.pow(10,Xmax)/3],
			'y': [Ymax/2, Ymax*3/4],
			'mode': 'markers',
			'marker': {
				'sizemode': 'area',
				'sizeref': 80,
				'size': [AvgZ,10*AvgZ],
				'color': 'rgb(255,255,255)',
				'line': {
					'width': 2,
					'color': 'rgb(0,0,255)',
					'dash': 'dot'
				}
			}
		}
		frame['data'].append(data_dict_abuse)

	# draw colorbar
		data_dict_colormap={
			'x': [],
			'y': [],
			'colorscale':[[0.0, 'rgb(0,0,0)'], [(10*AvgY-AvgY)*100/(Ymax-AvgY), 'rgb('+str((10*AvgY-AvgY)*255/(Ymax-AvgY))+',0,0)'], [100, 'rgb(255,0,0)']],
			'colorbar': {
				'title': 'Distance to average',
				'titleside': 'top',
				'tickmode': 'array',
				'tickvals': [0, (10*AvgY-AvgY)*100/(Ymax-AvgY), 100],
				'ticktext': ['Avg', '10x Avg', 'Max'],
				'ticks': 'outside'
			}
		}
		frame['data'].append(data_dict_colormap)
		

		figure['frames'].append(frame)
		
		slider_step = {'args': [
			[DataPoint.strip('-tlds.cvs')],
		{'frame': {'duration': 300, 'redraw': False},
		 'mode': 'immediate',
	       'transition': {'duration': 300}
			}
	     ],
	     'label': DataPoint.strip('-tlds.cvs'),
	     'method': 'animate'}
		sliders_dict['steps'].append(slider_step)

	    
	figure['layout']['sliders'] = [sliders_dict]
	plot(figure, filename=Abuse+".html")
